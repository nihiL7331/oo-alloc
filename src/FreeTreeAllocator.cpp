#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include "internal/RBTree.hpp"
#include <cassert>
#include <cstring>
#include <new>

namespace oo_alloc {

FreeTreeAllocator::~FreeTreeAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
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
void* FreeTreeAllocator::alloc(std::size_t size, std::size_t align) {
  if (align < alignof(void*))
    align = alignof(void*);
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

// thanks to the back_ptr,
// the time complexity of free is O(1).
void FreeTreeAllocator::free(void* ptr) {
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

  // insert new free block
  m_free_tree->insert(tree_node);
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

/* unlike the free-list implementation, this intentionally
 * avoids leftwards expansion. if the block can't be expanded
 * in place, it immediately goes to alloc -> memcpy -> free.
 *
 * because the red-black tree provides O(log n) search times,
 * finding a new block is fast, so it's a worthy trade-off
 * for the possible fragmentation.
 *
 * it's also how dlmalloc handles realloc.
 */ 
void* FreeTreeAllocator::realloc(void* ptr, std::size_t old_size, 
              std::size_t new_size, std::size_t align) {
  // need to align the data to
  // at least pointers alignment (8B default).
  // its enforced for multiple reasons,
  // most notably need enough alignment padding
  // before the payload to store the 'back_ptr'
  if (align < alignof(void *))
    align = alignof(void *);

  // if there's no data to reallocate,
  // instead just allocate
  if (ptr == nullptr)
    return alloc(new_size, align);

  // if the new_size is meant to be 0,
  // free the data
  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

  // step back into the alignment padding to read the `back_ptr`
  // and get original 'header_ptr'
  AllocHeader** back_ptr = reinterpret_cast<AllocHeader **>(ptr) - 1;
  AllocHeader* header_ptr = *back_ptr;

  // store data block size early before any changes
  std::size_t curr_size = header_ptr->size();

  std::uintptr_t header_addr = reinterpret_cast<std::uintptr_t>(header_ptr);
  std::uintptr_t req_end_addr = reinterpret_cast<std::uintptr_t>(ptr) + new_size;
  std::size_t req_size = utils::align_up(
    req_end_addr - header_addr - sizeof(AllocHeader),
    alignof(AllocHeader)
  );

  // if there's enough space already, just return
  // don't shrink because it's slower,
  // and its uncommon to shrink data via realloc
  if (req_size <= curr_size)
    return ptr;

  // get header of right neighbor block
  AllocHeader* right_header = reinterpret_cast<AllocHeader *>(
    reinterpret_cast<std::uint8_t *>(header_ptr) + sizeof(AllocHeader)
      + curr_size + sizeof(AllocFooter)
  );

  // if it's not allocated, check if the 'new_size'
  // can fit inside current block + its right neighbor
  if (!right_header->allocated()) {
    std::size_t right_size = right_header->size();
    std::size_t combo_size = curr_size + sizeof(AllocFooter) + sizeof(AllocHeader) + right_size;

    // if it can fit, expand to the right
    if (req_size <= combo_size) {
      internal::RBTree::Node* right_node = reinterpret_cast<internal::RBTree::Node *>(right_header + 1);
      m_free_tree->remove(right_node);

      constexpr std::size_t min_split_size = sizeof(AllocHeader) +
        sizeof(internal::RBTree::Node) + sizeof(AllocFooter);

      // if there's enough space left after expansion,
      // make a new free block
      if (combo_size - req_size >= min_split_size) {
        update_block(header_ptr, req_size, true);

        AllocHeader* free_header_ptr = reinterpret_cast<AllocHeader *>(
          reinterpret_cast<std::uint8_t *>(header_ptr) +
            sizeof(AllocHeader) + req_size + sizeof(AllocFooter)
        );

        std::size_t free_size = combo_size - req_size - 
          sizeof(AllocHeader) - sizeof(AllocFooter);
        update_block(free_header_ptr, free_size, false);

        internal::RBTree::Node* new_free_node = reinterpret_cast<internal::RBTree::Node *>(free_header_ptr + 1);
        new_free_node->size = free_size;
        new_free_node->parent = m_free_tree->sentinel();
        new_free_node->left = m_free_tree->sentinel();
        new_free_node->right = m_free_tree->sentinel();
        new_free_node->red = true;

        m_free_tree->insert(new_free_node);
      } else {
        // otherwise just consume the right neighbor fully
        update_block(header_ptr, combo_size, true);
      }
      return ptr;
    }
  }

  // if it can't fit in its right neighbor,
  // just allocate new data and copy it there,
  // after free the old data
  void *new_ptr = alloc(new_size, align);
  if (new_ptr != nullptr) {
    std::size_t copy_size = (old_size < new_size) ? old_size : new_size;
    std::memcpy(new_ptr, ptr, copy_size);
    this->free(ptr);
  }

  return new_ptr;
}

}
