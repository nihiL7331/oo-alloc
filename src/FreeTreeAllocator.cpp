#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include "internal/RBTree.hpp"
#include <cassert>
#include <cstring>
#include <new>

namespace oo_alloc {

FreeTreeAllocator::FreeTreeAllocator(std::size_t size)
  : m_start_ptr(nullptr)
  , m_total_size(0)
  , m_free_tree(nullptr) {
  if (size == 0)
    return;

  m_total_size = utils::align_up(size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr) {
    m_total_size = 0;
    return;
  }

  m_free_tree = new (m_start_ptr) internal::RBTree();

  this->clear();
}

FreeTreeAllocator::~FreeTreeAllocator() {
  if (m_start_ptr == nullptr)
    return;

  // created with placement new,
  // need to call the destructor
  if (m_free_tree != nullptr)
    m_free_tree->~RBTree();

  utils::os_free(m_start_ptr, m_total_size);

  m_start_ptr = nullptr;
  m_free_tree = nullptr;
}

/* coalescing is responsible for merging neighboring free blocks
 * together into a one bigger block. thanks to the structure of
 * this allocator, it's a O(log n) time implementation (due to tree removal)
 */
FreeTreeAllocator::AllocHeader* FreeTreeAllocator::coalesce(AllocHeader* header_ptr) {
  // store 'curr_size' early to 
  // not read overridden data later
  std::size_t curr_size = header_ptr->size();

  // get footer of the left neighbor
  AllocFooter* left_footer = reinterpret_cast<AllocFooter *>(
    reinterpret_cast<std::uint8_t *>(header_ptr) - sizeof(AllocFooter)
  );

  // get header of the right neighbor
  AllocHeader* right_header = reinterpret_cast<AllocHeader *>(
    reinterpret_cast<std::uint8_t *>(header_ptr) 
    + sizeof(AllocHeader) + header_ptr->size() + sizeof(AllocFooter)
  );

  // we dont need to check for out of bounds here.
  // 'clear()' places prologue and epilogue at the
  // edges of the memory, which when are checked with
  // 'allocated()' return true.

  // first we expand to the right, 
  // so that the pointer stands in place for left expand
  if(!right_header->allocated()) {
    // grab 'right_header's node
    internal::RBTree::Node* right_node = reinterpret_cast<internal::RBTree::Node *>(right_header + 1);
    m_free_tree->remove(right_node);

    // add the space taken by 'right_node' + metadata to the size
    curr_size += sizeof(AllocHeader) + right_header->size() + sizeof(AllocFooter);
  }

  // then we expand to the left.
  if(!left_footer->allocated()) {
    // grab 'left_footer's node
    AllocHeader* left_header = reinterpret_cast<AllocHeader *>(
      reinterpret_cast<std::uint8_t *>(left_footer) - left_footer->size() - sizeof(AllocHeader)
    );

    // remove the left node
    internal::RBTree::Node* left_node = reinterpret_cast<internal::RBTree::Node *>(left_header + 1);
    m_free_tree->remove(left_node);

    // add the space taken by 'left_node' + metadata to the size
    curr_size += sizeof(AllocHeader) + left_header->size() + sizeof(AllocFooter);

    // shift the header of coalesced block leftwards
    header_ptr = left_header;
  }

  // update the block size and place footer in correct position
  update_block(header_ptr, curr_size, false);

  return header_ptr;
}

/* memory layout
 * if allocated: [ header ] [ data ] [ footer ]
 * if free:      [ header ] [ tree node ] [ garbage ] [ footer ]
 */
void* FreeTreeAllocator::alloc_raw(std::size_t size, std::size_t align) {
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  if (align < alignof(void*))
    align = alignof(void*);

  if (size == 0 || size > SIZE_MAX - align - sizeof(AllocHeader*))
    return nullptr;

  // calculate worst case size
  std::size_t search_size = size + (align - 1) + sizeof(AllocHeader*);
  search_size = utils::align_up(search_size, alignof(AllocHeader));

  constexpr std::size_t node_size = sizeof(internal::RBTree::Node);
  if (search_size < node_size)
    search_size = utils::align_up(node_size, alignof(AllocHeader));

  // find the free node (best-fit)
  internal::RBTree::Node* node = m_free_tree->find_best(search_size);

  // if no node is found, return
  if (node == m_free_tree->sentinel())
    return nullptr;

  std::size_t orig_size = node->size;

  // remove the free node from the tree
  m_free_tree->remove(node);

  // get start of data
  std::uintptr_t raw_ptr = reinterpret_cast<std::uintptr_t>(node);

  // get header of found block
  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(raw_ptr - sizeof(AllocHeader));

  // get aligned pointer
  std::uintptr_t min_align_ptr = raw_ptr + sizeof(AllocHeader*);
  std::uintptr_t align_ptr = utils::align_up(min_align_ptr, align);

  // this uses the padding to store a pointer to the header.
  // 'align_ptr' might be pushed any distance from 'header_ptr'.
  // hence, we cant do 'ptr - sizeof(AllocHeader)' in 'free()',
  // because there's padding in between.
  // that's why align is enforced to be at least equal to the
  // pointer alignment, so that the pointer to the header
  // can be hidden inside of the padding.
  AllocHeader** back_ptr = reinterpret_cast<AllocHeader**>(align_ptr) - 1;
  *back_ptr = header_ptr;

  // if the block is way bigger than requested
  // size, split it in two and insert the leftover
  // back to free tree.
  constexpr std::size_t min_split_size = sizeof(AllocHeader) + sizeof(AllocFooter) + node_size;
  if (orig_size - search_size >= min_split_size) {
    // update the data block
    update_block(header_ptr, search_size, true);

    // get free block pointer
    AllocHeader* free_header_ptr = reinterpret_cast<AllocHeader *>(
      reinterpret_cast<uint8_t *>(
        header_ptr
      ) + sizeof(AllocHeader) + search_size + sizeof(AllocFooter)
    );

    // calculate the size of free block and update it
    std::size_t free_size = orig_size - search_size - sizeof(AllocHeader) - sizeof(AllocFooter);
    update_block(free_header_ptr, free_size, false);

    // insert free block to tree
    internal::RBTree::Node* tree_node = reinterpret_cast<internal::RBTree::Node *>(free_header_ptr + 1);
    tree_node->size = free_size;
    tree_node->parent = m_free_tree->sentinel();
    tree_node->left = m_free_tree->sentinel();
    tree_node->right = m_free_tree->sentinel();
    tree_node->red = true;
    m_free_tree->insert(tree_node);

    // no need to coalesce, we know that there is no
    // free block neighboring
  } else {
    // otherwise, just give the whole block
    update_block(header_ptr, orig_size, true);
  }

  // return the data pointer
  return reinterpret_cast<void *>(align_ptr);
}

// thanks to the back_ptr,
// the time complexity of free is O(1).
void FreeTreeAllocator::free_raw(void* ptr) {
  if (ptr == nullptr)
    return;

  // read back pointer
  AllocHeader** back_ptr = reinterpret_cast<AllocHeader **>(ptr) - 1;

  // dereference 'back_ptr' to get header pointer
  AllocHeader* header_ptr = *back_ptr;

  // mark the block as free
  update_block(header_ptr, header_ptr->size(), false);

  // coalesce with neighbors
  AllocHeader* free_header_ptr = coalesce(header_ptr);
  
  // create free block structure
  internal::RBTree::Node* tree_node = reinterpret_cast<internal::RBTree::Node *>(free_header_ptr + 1);
  tree_node->size = free_header_ptr->size();
  tree_node->parent = m_free_tree->sentinel();
  tree_node->left = m_free_tree->sentinel();
  tree_node->right = m_free_tree->sentinel();
  tree_node->red = true;

  // insert new free block
  m_free_tree->insert(tree_node);
}

void FreeTreeAllocator::clear() {
  // clear the tree
  m_free_tree->clear();

  std::uint8_t* start_ptr = reinterpret_cast<std::uint8_t *>(m_start_ptr);
  std::uint8_t* end_ptr = start_ptr + m_total_size;

  // set up prologue
  // it's a fake footer before the memory area,
  // used to stop left-coalescing on left-most block.
  AllocFooter* prologue_ptr = reinterpret_cast<AllocFooter *>(start_ptr + sizeof(internal::RBTree));
  prologue_ptr->state(0, true);

  // set up epilogue
  // it's a fake header, similar to prologue
  // stops right-coalescing on right-most block.
  AllocHeader* epilogue_ptr = reinterpret_cast<AllocHeader *>(end_ptr - sizeof(AllocHeader));
  epilogue_ptr->state(0, true);

  // calculate remaining space.
  // in init we assured that its enough
  // to contain a RBTree::Node.
  std::uint8_t* first_block_start = reinterpret_cast<std::uint8_t *>(prologue_ptr) + sizeof(AllocFooter);
  std::size_t total_remain_space = reinterpret_cast<std::uint8_t *>(epilogue_ptr) - first_block_start;
  std::size_t data_remain_space = total_remain_space - sizeof(AllocHeader) - sizeof(AllocFooter);
  
  // setup header and footer
  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(first_block_start);
  update_block(header_ptr, data_remain_space, false);

  // cast payload space to a r-b tree node
  internal::RBTree::Node* tree_node = reinterpret_cast<internal::RBTree::Node *>(header_ptr + 1);
  tree_node->size = data_remain_space;
  tree_node->parent = m_free_tree->sentinel();
  tree_node->left = m_free_tree->sentinel();
  tree_node->right = m_free_tree->sentinel();
  tree_node->red = true;

  // insert block to tree
  m_free_tree->insert(tree_node);
}

bool FreeTreeAllocator::owns(void* ptr) const {
  std::uintptr_t cast_ptr = reinterpret_cast<std::uintptr_t>(ptr);
  std::uintptr_t start_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);

  return cast_ptr >= start_ptr && cast_ptr < start_ptr + m_total_size;
}

}
