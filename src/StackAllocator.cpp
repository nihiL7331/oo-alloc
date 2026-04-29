#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace oo_alloc {

StackAllocator::StackAllocator()
  : m_start_ptr(nullptr), m_offset(0), m_total_size(0) {}

StackAllocator::~StackAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

void* StackAllocator::alloc(std::size_t size, std::size_t align) {
  std::uintptr_t base_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);
  std::uintptr_t curr_ptr = base_ptr + m_offset;

  std::size_t header_align = alignof(std::size_t);
  std::size_t header_size = sizeof(std::size_t);

  std::size_t actual_align = std::max(align, header_align);

  std::uintptr_t unalign_ptr = curr_ptr + header_size;
  std::size_t pad = utils::calc_pad(unalign_ptr, actual_align);
  std::uintptr_t data_ptr = unalign_ptr + pad;

  std::size_t alloc_size = header_size + pad + size;
  if (m_offset + alloc_size > m_total_size)
    return nullptr;

  std::size_t *header_ptr = reinterpret_cast<std::size_t *>(data_ptr - header_size);
  *header_ptr = m_offset;

  m_offset += alloc_size;

  return reinterpret_cast<void *>(data_ptr);
}

void StackAllocator::free(void* ptr) {
  if (ptr == nullptr)
    return;

  std::size_t target_offset = *(reinterpret_cast<std::size_t *>(ptr) - 1);
  m_offset = target_offset;
}

bool StackAllocator::init(std::size_t size) {
  m_total_size = utils::align_up(size, utils::page_size());
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  m_offset = 0;

  return true;
}

void StackAllocator::clear() {
  m_offset = 0;
}

void *StackAllocator::realloc(void *ptr, std::size_t old_size,
                              std::size_t new_size, std::size_t align) {
  if (new_size <= old_size)
    return ptr;

  std::uint8_t *raw_mem_ptr = reinterpret_cast<std::uint8_t *>(ptr);
  std::uint8_t *raw_start_ptr = reinterpret_cast<std::uint8_t *>(m_start_ptr);

  // only do anything if ptr is the last elem
  if (raw_start_ptr + m_offset == raw_mem_ptr + old_size) {
    std::size_t new_offset = m_offset + new_size - old_size;
    if (new_offset > m_total_size)
      return nullptr;

    m_offset = new_offset;

    return ptr;
  } else {
    void* new_ptr = this->alloc(new_size, align);

    if (new_ptr != nullptr)
      std::memcpy(new_ptr, ptr, old_size);

    return new_ptr;
  } 
}

}
