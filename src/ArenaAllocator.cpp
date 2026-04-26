#include "ArenaAllocator.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace oo_alloc {

ArenaAllocator::ArenaAllocator()
  : m_start_ptr(nullptr), m_total_size(0), m_offset(0) {}

ArenaAllocator::~ArenaAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

void* ArenaAllocator::alloc(std::size_t size, std::uint8_t align) {
  uintptr_t base_ptr = reinterpret_cast<uintptr_t>(m_start_ptr);
  uintptr_t curr_ptr = base_ptr + m_offset;

  uintptr_t pad = (align - (curr_ptr & (align - 1))) & (align - 1);

  if (m_offset + pad + size > m_total_size)
    return nullptr;

  uintptr_t align_ptr = curr_ptr + pad;

  m_offset += (pad + size);

  return reinterpret_cast<void *>(align_ptr);
}

void ArenaAllocator::free(void* ptr) {
  (void)ptr;
}

bool ArenaAllocator::init(std::size_t size) {
  m_start_ptr = std::malloc(size);
  if (m_start_ptr == nullptr)
    return false;

  m_total_size = size;
  m_offset = 0;
  return true;
}

void ArenaAllocator::clear() {
  m_offset = 0;
}
   
}
