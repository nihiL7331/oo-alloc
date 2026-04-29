#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace oo_alloc {

PoolAllocator::~PoolAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

void* PoolAllocator::alloc(std::size_t size, std::size_t align) {
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

  m_total_size = utils::align_up(size, utils::page_size());
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  clear();

  return true;
}

void PoolAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

  std::uint8_t* raw_mem_ptr = reinterpret_cast<std::uint8_t*>(m_start_ptr);
  std::size_t num_chunks = m_total_size / m_chunk_size;

  for (std::size_t i = 0; i < num_chunks - 1; ++i) {
    void** curr_chunk = reinterpret_cast<void **>(raw_mem_ptr + (i * m_chunk_size));
    void* next_chunk = raw_mem_ptr + ((i + 1) * m_chunk_size);

    *curr_chunk = next_chunk;
  }

  void** last_chunk = reinterpret_cast<void **>(raw_mem_ptr + ((num_chunks - 1) * m_chunk_size));
  *last_chunk = nullptr;

  m_free_list_head = m_start_ptr;
}
   
void *PoolAllocator::realloc(void *ptr, std::size_t old_size,
                              std::size_t new_size, std::size_t align) {
  (void)old_size;
  (void)align;
  if (new_size > m_chunk_size)
    return nullptr;
  else
    return ptr;
}

}
