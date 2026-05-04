#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace oo_alloc {

FreeListAllocator::FreeListAllocator(std::size_t size) 
  : m_start_ptr(nullptr)
  , m_total_size(0)
  , m_free_list_head(nullptr) {
  if (size == 0)
    return;

  m_total_size = utils::align_up(size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr) {
    m_total_size = 0;
    return;
  }

  this->clear();
}

FreeListAllocator::~FreeListAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

// attempts to merge a freed block with its neighbors to reduce fragmentation.
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
 *
 * requires a list traversal to find a block to allocate in.
 */
void* FreeListAllocator::alloc_raw(std::size_t size, std::size_t align) {
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  if (size == 0 || size > SIZE_MAX - align - sizeof(AllocHeader))
    return nullptr;

  constexpr std::size_t header_size = sizeof(AllocHeader);

  FreeBlock* prev_ptr = nullptr;
  FreeBlock* curr_ptr = m_free_list_head;

  while (curr_ptr != nullptr) {
    std::uintptr_t curr_addr = reinterpret_cast<std::uintptr_t>(curr_ptr);
    std::uintptr_t unalign_ptr = curr_addr + header_size;

    std::size_t pad = utils::calc_pad(unalign_ptr, align);

    std::size_t required_size = utils::align_up(size + header_size + pad, alignof(FreeBlock));

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

void  FreeListAllocator::free_raw(void* ptr) {
  if (ptr == nullptr)
    return;

  constexpr std::size_t header_size = sizeof(AllocHeader);

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

// initializes the whole memory as a large free block
void FreeListAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

  FreeBlock* start_ptr = reinterpret_cast<FreeBlock *>(m_start_ptr);
  start_ptr->size = m_total_size;
  start_ptr->next = nullptr;
  m_free_list_head = start_ptr;
}

}
