#include "oo_alloc/PoolAllocator.hpp"
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
  if (ptr == nullptr)
    return;

  void** ret_chunk = reinterpret_cast<void **>(ptr);
  *ret_chunk = m_free_list_head;
  m_free_list_head = ptr;
}

bool PoolAllocator::init(std::size_t size) {
  if (size < m_chunk_size)
    return false;
  
  if (size % m_chunk_size != 0)
    return false;

  m_start_ptr = std::malloc(size);
  if (m_start_ptr == nullptr)
    return false;

  m_total_size = size;

  clear();

  return true;
}

void PoolAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

  uint8_t* raw_mem = reinterpret_cast<uint8_t*>(m_start_ptr);
  std::size_t num_chunks = m_total_size / m_chunk_size;

  for (std::size_t i = 0; i < num_chunks - 1; ++i) {
    void** curr_chunk = reinterpret_cast<void **>(raw_mem + (i * m_chunk_size));
    void* next_chunk = raw_mem + ((i + 1) * m_chunk_size);

    *curr_chunk = next_chunk;
  }

  void** last_chunk = reinterpret_cast<void **>(raw_mem + ((num_chunks - 1) * m_chunk_size));
  *last_chunk = nullptr;

  m_free_list_head = m_start_ptr;
}
   
}
