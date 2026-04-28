#include "oo_alloc/FreeListAllocator.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>

namespace oo_alloc {

FreeListAllocator::FreeListAllocator()
  : m_start_ptr(nullptr), m_total_size(0), m_free_list_head(nullptr) {}

FreeListAllocator::~FreeListAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

/* memory structure: 
 * pad       | header | data        (when allocated)
 * free_size | next*  | empty space (when not allocated) 
 */
void* FreeListAllocator::alloc(std::size_t size, std::uint8_t align) {
  std::size_t header_size = sizeof(AllocHeader);

  FreeBlock* prev_ptr = nullptr;
  FreeBlock* curr_ptr = m_free_list_head;

  while (curr_ptr != nullptr) {
    std::uintptr_t curr_addr = reinterpret_cast<std::uintptr_t>(curr_ptr);
    std::uintptr_t unalign_ptr = curr_addr + header_size;

    std::uint8_t pad = (align - (unalign_ptr & (align - 1))) & (align - 1);

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
  std::uint8_t pad = header_ptr->pad;
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

  m_start_ptr = std::malloc(size);
  if (m_start_ptr == nullptr)
    return false;

  m_total_size = size;

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
                              std::size_t new_size, std::uint8_t align) {
  (void)ptr;
  (void)old_size;
  (void)new_size;
  (void)align;
  assert(false && "TODO");
  return nullptr;
}

}
