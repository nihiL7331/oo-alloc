#include "StackAllocator.hpp"
#include <algorithm>
#include <cstdlib>

namespace oo_alloc {

StackAllocator::StackAllocator()
  : m_start_ptr(nullptr), m_offset(0), m_total_size(0) {}

StackAllocator::~StackAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

void* StackAllocator::alloc(std::size_t size, std::uint8_t align) {
  uintptr_t base_ptr = reinterpret_cast<uintptr_t>(m_start_ptr);
  uintptr_t curr_ptr = base_ptr + m_offset;

  std::uint8_t header_align = alignof(std::size_t);
  std::uint8_t header_size = sizeof(std::size_t);

  std::uint8_t actual_align = std::max(align, header_align);

  uintptr_t unalign_ptr = curr_ptr + header_size;
  std::uint8_t pad = (actual_align - (unalign_ptr & (actual_align - 1))) & (actual_align - 1);
  uintptr_t data_ptr = unalign_ptr + pad;

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
  m_start_ptr = std::malloc(size);
  if (m_start_ptr == nullptr)
    return false;

  m_total_size = size;
  m_offset = 0;
  return true;
}

void StackAllocator::clear() {
  m_offset = 0;
}

}
