#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace oo_alloc {

FreeListAllocator::~FreeListAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

void FreeListAllocator::coalesce(FreeBlock* prev_block, FreeBlock* free_block) {
  FreeBlock* next_block = free_block->next;

  // merge right
  if (next_block != nullptr) {
    std::uint8_t* free_ptr = reinterpret_cast<std::uint8_t*>(free_block);
    std::uint8_t* next_ptr = reinterpret_cast<std::uint8_t*>(next_block);
    
    if (free_ptr + free_block->size == next_ptr) {
      free_block->size += next_block->size;
      free_block->next = next_block->next;
    }
  }

  // merge left
  if (prev_block != nullptr) {
    std::uint8_t* prev_ptr = reinterpret_cast<std::uint8_t*>(prev_block);
    std::uint8_t* free_ptr = reinterpret_cast<std::uint8_t*>(free_block);
    
    if (prev_ptr + prev_block->size == free_ptr) {
      prev_block->size += free_block->size;
      prev_block->next = free_block->next;
    }
  }
}

/* memory structure: 
 * pad       | header | data        (when allocated)
 * free_size | next*  | empty space (when not allocated) 
 */
void* FreeListAllocator::alloc(std::size_t size, std::size_t align) {
  std::size_t header_size = sizeof(AllocHeader);

  FreeBlock* prev_ptr = nullptr;
  FreeBlock* curr_ptr = m_free_list_head;

  while (curr_ptr != nullptr) {
    std::uintptr_t curr_addr = reinterpret_cast<std::uintptr_t>(curr_ptr);
    std::uintptr_t unalign_ptr = curr_addr + header_size;

    std::size_t pad = utils::calc_pad(unalign_ptr, align);

    std::size_t required_size = size + header_size + pad;
    std::size_t free_size = curr_ptr->size;

    // 1. find address for the data
    if (free_size >= required_size) {
      // block found
      FreeBlock* new_block = nullptr;

      if (free_size - required_size >= sizeof(FreeBlock)) {
        // create new free data block from left data
        std::uint8_t* raw_addr = reinterpret_cast<std::uint8_t *>(curr_ptr) + required_size;
        new_block = reinterpret_cast<FreeBlock *>(raw_addr);

        new_block->size = free_size - required_size;
        new_block->next = curr_ptr->next;
      } else {
        // consume whole block
        new_block = curr_ptr->next;
        required_size = free_size;
      }

      if (prev_ptr == nullptr)
        m_free_list_head = new_block;
      else
        prev_ptr->next = new_block;

    // 2. place header behind it
      std::uint8_t* header_ptr = reinterpret_cast<std::uint8_t *>(curr_ptr) + pad;
      AllocHeader* header = reinterpret_cast<AllocHeader *>(header_ptr);
      header->size = required_size;
      header->pad = pad;

      std::uint8_t* data_ptr = header_ptr + header_size;
      return static_cast<void *>(data_ptr);
    }

    prev_ptr = curr_ptr;
    curr_ptr = curr_ptr->next;
  }

  return nullptr;
}

void  FreeListAllocator::free(void* ptr) {
  if (ptr == nullptr)
    return;

  std::size_t header_size = sizeof(AllocHeader);

  // step backwards to get the header
  std::uint8_t* data_ptr = reinterpret_cast<std::uint8_t *>(ptr);
  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(data_ptr - header_size);

  // step backwards again to origin
  std::size_t size = header_ptr->size;
  std::size_t pad = header_ptr->pad;
  std::uint8_t* orig_ptr = reinterpret_cast<std::uint8_t *>(header_ptr) - pad;
  FreeBlock* new_free_ptr = reinterpret_cast<FreeBlock *>(orig_ptr);
  new_free_ptr->size = size;

  // insert back to free list

  // find position
  FreeBlock* prev_ptr = nullptr;
  FreeBlock* curr_ptr = m_free_list_head;
  while(curr_ptr != nullptr && curr_ptr < new_free_ptr) {
    prev_ptr = curr_ptr;
    curr_ptr = curr_ptr->next;
  }

  // insert
  if (prev_ptr != nullptr) {
    prev_ptr->next = new_free_ptr;
    new_free_ptr->next = curr_ptr;
  } else { // free is head
    new_free_ptr->next = curr_ptr;
    m_free_list_head = new_free_ptr;
  }

  // coalescence
  coalesce(prev_ptr, new_free_ptr);
}

bool FreeListAllocator::init(std::size_t size) {
  if (size < sizeof(FreeBlock)) // too small to allocate anything
    return false;

  m_total_size = utils::align_up(size, utils::page_size());
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  clear();

  return true;
}

// initializes the whole memory as a large free block
void FreeListAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

  FreeBlock* start_ptr = reinterpret_cast<FreeBlock *>(m_start_ptr);
  start_ptr->size = m_total_size;
  start_ptr->next = nullptr;
  m_free_list_head = start_ptr;
}

void* FreeListAllocator::shrink_in_place(void* ptr, AllocHeader* header, std::size_t old_size, std::size_t align_new_size) {
  std::uint8_t *raw_ptr = static_cast<std::uint8_t *>(ptr);
  header->size = align_new_size;

  if (old_size - align_new_size >= sizeof(FreeBlock)) {
    FreeBlock *free_ptr = reinterpret_cast<FreeBlock *>(raw_ptr + align_new_size);
    free_ptr->size = old_size - align_new_size;

    FreeBlock *prev_ptr = nullptr;
    FreeBlock *curr_ptr = m_free_list_head;
    while (curr_ptr != nullptr && curr_ptr < free_ptr) {
      prev_ptr = curr_ptr;
      curr_ptr = curr_ptr->next;
    }

    if (prev_ptr == nullptr)
      m_free_list_head = free_ptr;
    else
      prev_ptr->next = free_ptr;
    free_ptr->next = curr_ptr;
  }

  return ptr;
}

void* FreeListAllocator::expand_right(void* ptr, AllocHeader* header, std::size_t old_size, 
                     std::size_t align_new_size, FreeBlock* right_block, FreeBlock* right_prev) {
  std::size_t size_diff = align_new_size - old_size;
  std::uint8_t* curr_cast_ptr = reinterpret_cast<std::uint8_t *>(right_block);

  if (right_block->size - size_diff < sizeof(FreeBlock)) {
    if (right_prev == nullptr)
      m_free_list_head = right_block->next;
    else
      right_prev->next = right_block->next;
  } else {
    FreeBlock *new_curr_ptr =
        reinterpret_cast<FreeBlock *>(curr_cast_ptr + size_diff);
    new_curr_ptr->size = right_block->size - size_diff;
    new_curr_ptr->next = right_block->next;

    if (right_prev == nullptr)
      m_free_list_head = new_curr_ptr;
    else
      right_prev->next = new_curr_ptr;
  }

  header->size = align_new_size;
  return ptr;
}

void* FreeListAllocator::expand_left(void* ptr, AllocHeader* header, 
                                     std::size_t old_size, std::size_t align_new_size, 
                                     FreeBlock* left_block, FreeBlock* left_prev) {
  std::size_t size_diff = align_new_size - old_size;
  std::uint8_t* proposed_new_ptr = static_cast<std::uint8_t *>(ptr) - size_diff;

  // if the left block is too small to split,
  // consume the whole block
  if (left_block->size - size_diff < sizeof(FreeBlock)) {
    if (left_prev != nullptr)
      left_prev->next = left_block->next;
    else
      m_free_list_head = left_block->next;

    // shift left by full size of the block
    std::uint8_t* new_ptr = static_cast<std::uint8_t *>(ptr) - left_block->size;

    // write new metadata
    AllocHeader* new_header = get_header(new_ptr);
    new_header->size = old_size + left_block->size;
    new_header->pad = header->pad;

    std::memmove(new_ptr, ptr, old_size);
    return new_ptr;
  } else {
    // left block is big enough to split,
    // so just shrink the left block
    left_block->size -= size_diff;
    
    // write new metadata
    AllocHeader* new_header = get_header(proposed_new_ptr);
    new_header->size = align_new_size;
    new_header->pad = header->pad;
    
    std::memmove(proposed_new_ptr, ptr, old_size);
    return proposed_new_ptr;
  }
}

void* FreeListAllocator::expand_both(void* ptr, AllocHeader* header, 
                                     std::size_t old_size, std::size_t align_new_size, 
                                     FreeBlock* left_block, FreeBlock* left_prev, 
                                     FreeBlock* right_block) {
  std::size_t size_diff = align_new_size - old_size;
  std::size_t right_size_diff = size_diff - left_block->size;

  std::uint8_t* proposed_new_ptr = static_cast<std::uint8_t *>(ptr) - left_block->size;

  // the remainder of the right block is too small,
  // consume both the left and the right block
  if (right_block->size - right_size_diff < sizeof(FreeBlock)) {
    // bypass both blocks
    if (left_prev != nullptr)
      left_prev->next = right_block->next;
    else
      m_free_list_head = right_block->next;

    // write new metadata
    AllocHeader* new_header = get_header(proposed_new_ptr);
    new_header->size = old_size + left_block->size + right_block->size;
    new_header->pad = header->pad;

    std::memmove(proposed_new_ptr, ptr, old_size);
    return proposed_new_ptr;
  } else {
    // the right block is big enough to be split,
    // consume the left block, shrink the right

    // create new, smaller block shifted to the right
    std::uint8_t* right_cast_ptr = reinterpret_cast<std::uint8_t *>(right_block);
    FreeBlock* new_right_block = reinterpret_cast<FreeBlock *>(right_cast_ptr + right_size_diff);

    new_right_block->size = right_block->size - right_size_diff;
    new_right_block->next = right_block->next;

    // bypass the 'left_block', go to the 'new_right_block'
    if (left_prev != nullptr)
      left_prev->next = new_right_block;
    else
      m_free_list_head = new_right_block;

    // write new metadata
    AllocHeader* new_header = get_header(proposed_new_ptr);
    new_header->size = align_new_size;
    new_header->pad = header->pad;

    std::memmove(proposed_new_ptr, ptr, old_size);
    return proposed_new_ptr;
  }

  return proposed_new_ptr;
}

/*
 / NOTE: this implementation isn't fast.
 / if the block has to left-expand, it has to use std::memmove
 / which is slow.
 / if it can right-expand, it's faster, but still requires
 / list traversal.
 / if it can't do either, it has to either:
 / - allocate a completely fresh chunk of memory 
 /   and call std::copy, as well as a free, or
 / - do the combination of left&right expand,
 /   which wastes less memory but is slower.
 /
 / it focuses on minimizing the fragmentation, contrary
 / to the free tree implementation. 
 */
void *FreeListAllocator::realloc(void *ptr, std::size_t old_size,
                                 std::size_t new_size, std::size_t align) {
  if (ptr == nullptr)
    return alloc(new_size, align);
  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

  AllocHeader* header_ptr = get_header(ptr);
  std::size_t align_new_size = utils::align_up(new_size, align);

  // case 1: new size is smaller, change in place
  // optionally create a new free block, coalesce it
  if (align_new_size <= old_size)
    return shrink_in_place(ptr, header_ptr, old_size, align_new_size);

  std::uint8_t* raw_ptr = static_cast<std::uint8_t *>(ptr);
  FreeBlock* prev_prev_ptr = nullptr;
  FreeBlock* left_neighbor = nullptr;
  FreeBlock* right_neighbor = m_free_list_head;

  while (right_neighbor != nullptr && reinterpret_cast<std::uint8_t *>(right_neighbor) < raw_ptr) {
    prev_prev_ptr = left_neighbor;
    left_neighbor = right_neighbor;
    right_neighbor = right_neighbor->next;
  }

  bool can_expand_right = touches_right(ptr, old_size, right_neighbor) && 
                          (right_neighbor->size >= align_new_size - old_size);

  bool can_expand_left = touches_left(left_neighbor, header_ptr) &&
                        (left_neighbor->size >= align_new_size - old_size);

  bool can_expand_both = touches_left(left_neighbor, header_ptr) &&
                        touches_right(ptr, old_size, right_neighbor) &&
                        (left_neighbor->size + right_neighbor->size >= align_new_size - old_size);

  // case 2: there's a free block directly after reallocated memory,
  // it has enough space, do a right-expand
  if (can_expand_right)
    return expand_right(ptr, header_ptr, old_size, align_new_size, right_neighbor, left_neighbor);
  

  // case 3: there's a free block directly before the allocated memory,
  // it has enough space and the proposed ptr is correctly aligned, do a left-expand
  // otherwise fallback to new alloc
  if (can_expand_left) {
    std::size_t size_diff = align_new_size - old_size;
    void* proposed_new_ptr = raw_ptr - size_diff;

    if (aligned(proposed_new_ptr, align))
      return expand_left(ptr, header_ptr, old_size, align_new_size, left_neighbor, prev_prev_ptr);
  }

  // case 4: there are free block directly before AND after the allocated memory,
  // they together can fit the reallocated memory, do a left-shift, right-expand
  if (can_expand_both) {
    void* proposed_new_ptr = raw_ptr - left_neighbor->size; // shift left as much as possible

    if (aligned(proposed_new_ptr, align))
      return expand_both(ptr, header_ptr, old_size, align_new_size, left_neighbor, prev_prev_ptr, right_neighbor);
  }

  // case 5: there's not enough free memory neighboring, 
  // need to allocate a new block and copy the data.
  // we also free the old data
  void *new_ptr = alloc(align_new_size, align);
  if (new_ptr != nullptr) {
    std::memcpy(new_ptr, ptr, old_size);
    this->free(ptr);
  }

  return new_ptr;
}

}
