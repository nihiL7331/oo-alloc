#include "PoolAllocator.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace oo_alloc {

PoolAllocator::~PoolAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

void* PoolAllocator::alloc(std::size_t size, std::uint8_t align) {
  if (size != m_chunk_size || align != m_chunk_align)
    return nullptr;
  if (m_free_list_head == nullptr)
    return nullptr;

  void* ret_head = m_free_list_head;
  m_free_list_head = *reinterpret_cast<void **>(m_free_list_head);

  return ret_head;
}

void PoolAllocator::free(void* ptr) {
}

bool PoolAllocator::init(std::size_t size) {
}

void PoolAllocator::clear() {
}
   
}
