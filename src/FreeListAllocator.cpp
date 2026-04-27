#include "oo_alloc/FreeListAllocator.hpp"
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
