#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include "internal/RBTree.hpp"
#include <cassert>
#include <new>

namespace oo_alloc {

FreeTreeAllocator::~FreeTreeAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

FreeTreeAllocator::AllocHeader* FreeTreeAllocator::coalesce(AllocHeader* header) {
  (void)header;
  assert(false && "TODO");
}

/* memory layout
 * if allocated: [ header ] [ data ] [ footer ]
 * if free:      [ header ] [ tree node ] [ garbage ] [ footer ]
 */
void* FreeTreeAllocator::alloc(std::size_t size, std::size_t align) {
  // calculate worst case size
  std::size_t search_size = size + (align - 1) + sizeof(AllocHeader*);
  search_size = utils::align_up(search_size, alignof(AllocHeader));

  constexpr std::size_t node_size = sizeof(internal::RBTree::Node);
  if (search_size < node_size)
    search_size = utils::align_up(node_size, alignof(AllocHeader));

  // find the free node (best-fit)
  internal::RBTree::Node* node = m_free_tree->find_best(search_size);

  // if no node is found, return
  if (node == nullptr)
    return nullptr;

  // get start of data
  std::uintptr_t raw_ptr = reinterpret_cast<std::uintptr_t>(node);

  // get header of found block
  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(raw_ptr - sizeof(AllocHeader));

  // get aligned pointer
  std::uintptr_t min_align_ptr = raw_ptr + sizeof(AllocHeader*);
  std::uintptr_t align_ptr = utils::align_up(min_align_ptr, align);

  // store back pointer in padding
  AllocHeader** back_ptr = reinterpret_cast<AllocHeader**>(align_ptr) - 1;
  *back_ptr = header_ptr;

  // remove the free node from the tree
  m_free_tree->remove(node);

  // if the block is way bigger than requested
  // size, split it in two and insert the leftover
  // back to free tree.
  constexpr std::size_t min_split_size = sizeof(AllocHeader) + sizeof(AllocFooter) + node_size;
  if (node->size - search_size >= min_split_size) {
    std::size_t orig_size = node->size;

    // update the data block
    update_block(header_ptr, search_size, true);

    // get free block pointer
    AllocHeader* free_header_ptr = reinterpret_cast<AllocHeader *>(
      reinterpret_cast<uint8_t *>(
        header_ptr
      ) + sizeof(AllocHeader) + search_size
    );

    // calculate the size of free block and update it
    std::size_t free_size = orig_size - search_size - sizeof(AllocHeader) - sizeof(AllocFooter);
    update_block(free_header_ptr, free_size, false);

    // insert free block to tree
    internal::RBTree::Node* tree_node = reinterpret_cast<internal::RBTree::Node *>(free_header_ptr + 1);
    tree_node->size = free_size;
    m_free_tree->insert(tree_node);

    // no need to coalesce, we know that there is no
    // free block neighboring
  } else {
    // otherwise, just give the whole block
    update_block(header_ptr, node->size, true);
  }

  // return the data pointer
  return reinterpret_cast<void *>(align_ptr);
}

void FreeTreeAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool FreeTreeAllocator::init(std::size_t size) {
  // this allocator needs additional data
  // for holding the red-black tree structure.
  // this data will be allocated on the beginning
  // of the initialized memory page.
  constexpr std::size_t min_size = sizeof(internal::RBTree)
    + sizeof(AllocHeader) + sizeof(internal::RBTree::Node)
    + sizeof(AllocFooter);
  // too small to allocate anything
  if (size < min_size) 
    return false;

  m_total_size = utils::align_up(size, utils::page_size());
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  m_free_tree = new (m_start_ptr) internal::RBTree();

  clear();

  return true;
}

void FreeTreeAllocator::clear() {
  // clear the tree
  m_free_tree->clear();

  // calculate where the actual data starts
  std::uint8_t* start_ptr = reinterpret_cast<std::uint8_t *>(m_start_ptr) + sizeof(internal::RBTree);

  // calculate remaining space.
  // in init we assured that its enough
  // to contain a RBTree::Node.
  std::size_t total_remain_space = m_total_size - sizeof(internal::RBTree);
  std::size_t data_remain_space = total_remain_space - sizeof(AllocHeader) - sizeof(AllocFooter);
  
  // setup header and footer
  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(start_ptr);
  update_block(header_ptr, data_remain_space, false);

  // cast payload space to a r-b tree node
  internal::RBTree::Node* tree_node = reinterpret_cast<internal::RBTree::Node *>(start_ptr + sizeof(AllocHeader));
  tree_node->size = data_remain_space;

  // insert block to tree
  m_free_tree->insert(tree_node);
}

void* FreeTreeAllocator::realloc(void* ptr, std::size_t old_size, 
              std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

}
