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
  // merge new with curr first to avoid mismatches if merging both

  // merge new with curr (next)
  if (curr_ptr != nullptr) {
    std::uint8_t* raw_curr_ptr = reinterpret_cast<std::uint8_t *>(curr_ptr);
    if (orig_ptr + size == raw_curr_ptr) {
      new_free_ptr->size += curr_ptr->size;
      new_free_ptr->next = curr_ptr->next;
    }
  }
  // merge prev with new
  if (prev_ptr != nullptr) {
    std::uint8_t* raw_prev_ptr = reinterpret_cast<std::uint8_t *>(prev_ptr);
    if (raw_prev_ptr + prev_ptr->size == orig_ptr) { 
      prev_ptr->size += new_free_ptr->size;
      prev_ptr->next = new_free_ptr->next;
    }
  }
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

void *FreeListAllocator::realloc(void *ptr, std::size_t old_size,
                                 std::size_t new_size, std::size_t align) {
  // NOTE: this implementation isn't fast.
  // if the block has to left-expand, it has to use std::memmove
  // which is slow.
  // if it can right-expand, it's faster, but still requires
  // list traversal.
  // if it can't do either, it has to either:
  // - allocate a completely fresh chunk of memory 
  //   and call std::copy, as well as a free, or
  // - do the combination of left&right expand,
  //   which wastes less memory but is slower
  if (ptr == nullptr)
    return alloc(new_size, align);
  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

  // case 1: new size is smaller, change in place
  // optionally create a new free block, coalesce it
  if (new_size <= old_size) {
    std::uint8_t *raw_ptr = static_cast<std::uint8_t *>(ptr);
    AllocHeader *header_ptr =
        reinterpret_cast<AllocHeader *>(raw_ptr - sizeof(AllocHeader));
    header_ptr->size = new_size;

    std::size_t align_new_size = utils::align_up(new_size, align);

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

  // find closest free memory blocks
  std::uint8_t *raw_ptr = static_cast<std::uint8_t *>(ptr);
  FreeBlock *prev_prev_ptr = nullptr; // needed for case 3/4
  FreeBlock *prev_ptr = nullptr;
  FreeBlock *curr_ptr = m_free_list_head;
  while (curr_ptr != nullptr &&
         reinterpret_cast<std::uint8_t *>(curr_ptr) < raw_ptr) {
    prev_prev_ptr = prev_ptr;
    prev_ptr = curr_ptr;
    curr_ptr = curr_ptr->next;
  }

  AllocHeader* header_ptr = reinterpret_cast<AllocHeader *>(raw_ptr - sizeof(AllocHeader));
  std::size_t align_new_size = utils::align_up(new_size, align);

  if (curr_ptr != nullptr) {
    bool touches_left = (prev_ptr != nullptr) &&
                        (reinterpret_cast<std::uint8_t *>(prev_ptr) + prev_ptr->size 
                        == reinterpret_cast<std::uint8_t *>(header_ptr));

    bool touches_right = (curr_ptr != nullptr) &&
                         (raw_ptr + old_size == reinterpret_cast<std::uint8_t *>(curr_ptr));


    std::uint8_t *curr_cast_ptr = reinterpret_cast<std::uint8_t *>(curr_ptr);
    std::size_t size_diff = align_new_size - old_size;

    if (touches_right && curr_ptr->size >= align_new_size - old_size) {
      // case 2: there's a free block directly after reallocated memory,
      // it has enough space, do a right-expand

      if (curr_ptr->size - size_diff < sizeof(FreeBlock)) {
        if (prev_ptr == nullptr)
          m_free_list_head = curr_ptr->next;
        else
          prev_ptr->next = curr_ptr->next;
      } else {
        FreeBlock *new_curr_ptr =
            reinterpret_cast<FreeBlock *>(curr_cast_ptr + size_diff);
        new_curr_ptr->size = curr_ptr->size - size_diff;
        new_curr_ptr->next = curr_ptr->next;

        if (prev_ptr == nullptr)
          m_free_list_head = new_curr_ptr;
        else
          prev_ptr->next = new_curr_ptr;
      }

      header_ptr->size = align_new_size;
      return ptr;
    } else if (touches_left && prev_ptr->size >= align_new_size - old_size) {
      // case 3: there's a free block directly before the allocated memory,
      // it has enough space and the proposed ptr is correctly aligned, do a left-expand
      // otherwise fallback to new alloc

      std::uint8_t* proposed_new_ptr = reinterpret_cast<std::uint8_t *>(ptr) - size_diff;

      if (reinterpret_cast<std::uintptr_t>(proposed_new_ptr) % align == 0) {
        if (prev_ptr->size - size_diff < sizeof(FreeBlock)) {
          if (prev_prev_ptr != nullptr)
            prev_prev_ptr->next = curr_ptr;
          else
            m_free_list_head = curr_ptr;

          std::uint8_t* new_ptr = reinterpret_cast<std::uint8_t *>(ptr) - prev_ptr->size;
          AllocHeader* new_header_ptr = reinterpret_cast<AllocHeader *>(new_ptr - sizeof(AllocHeader));
          new_header_ptr->size = old_size + prev_ptr->size;
          new_header_ptr->pad = header_ptr->pad;

          std::memmove(new_ptr, ptr, old_size);
          return new_ptr;
        } else {
          prev_ptr->size -= size_diff;
          
          AllocHeader* new_header_ptr = reinterpret_cast<AllocHeader *>(proposed_new_ptr - sizeof(AllocHeader));
          new_header_ptr->size = align_new_size;
          new_header_ptr->pad = header_ptr->pad;
          
          std::memmove(proposed_new_ptr, ptr, old_size);
          return proposed_new_ptr;
        }
      }
    } else if (touches_left && touches_right && prev_ptr->size + curr_ptr->size >= align_new_size - old_size) {
      // case 4: there are free block directly before AND after the allocated memory,
      // they together can fit the reallocated memory, do a left-shift, right-expand
      std::uint8_t* proposed_new_ptr = reinterpret_cast<std::uint8_t *>(ptr) - prev_ptr->size;

      if (reinterpret_cast<std::uintptr_t>(proposed_new_ptr) % align == 0) {
        std::size_t right_size_diff = size_diff - prev_ptr->size;

        // 1. create new header
        AllocHeader* new_header_ptr = reinterpret_cast<AllocHeader *>(proposed_new_ptr - sizeof(AllocHeader));
        new_header_ptr->pad = header_ptr->pad;

        // 2. move the data to new destination
        std::memmove(proposed_new_ptr, ptr, old_size);

        // 3. check if the right block will be deleted, if so just consume both blocks
        if (curr_ptr->size - right_size_diff < sizeof(FreeBlock)) {
          if (prev_prev_ptr == nullptr)
            m_free_list_head = curr_ptr->next;
          else
            prev_prev_ptr->next = curr_ptr->next;

          new_header_ptr->size = old_size + prev_ptr->size + curr_ptr->size;
        } else {
          // 4. consume left, shrink right 
          FreeBlock *new_curr_ptr =
              reinterpret_cast<FreeBlock *>(curr_cast_ptr + right_size_diff);
          new_curr_ptr->size = curr_ptr->size - right_size_diff;
          new_curr_ptr->next = curr_ptr->next;

          if (prev_prev_ptr == nullptr)
            m_free_list_head = new_curr_ptr;
          else
            prev_prev_ptr->next = new_curr_ptr;

          new_header_ptr->size = align_new_size;
        }

        return proposed_new_ptr;
      }
    }
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
