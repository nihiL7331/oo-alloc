#pragma once
#include "oo_alloc/IAllocator.hpp"
#include <algorithm>

namespace oo_alloc {

class PoolAllocator: public IAllocator {
private:
  std::size_t m_chunk_size;
  std::size_t m_chunk_align;
  void*       m_start_ptr;
  std::size_t m_total_size;
  void*       m_free_list_head;

public:
  PoolAllocator(std::size_t chunk_size, std::size_t chunk_align) {
    m_chunk_size = std::max(chunk_size, sizeof(void*));
    m_chunk_align = chunk_align;
    m_start_ptr = nullptr;
    m_total_size = 0;
    m_free_list_head = nullptr;
  }
  ~PoolAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
};

}
