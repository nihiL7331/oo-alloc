#pragma once
#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

class PoolAllocator: public IAllocator {
private:
  std::size_t m_chunk_size;
  std::size_t m_chunk_align;
  void*       m_start_ptr;
  std::size_t m_total_size;
  void*       m_free_list_head;

public:
  explicit PoolAllocator(std::size_t chunk_size, std::size_t chunk_align, std::size_t chunk_count);
  ~PoolAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  void clear() override;
  std::size_t capacity() const override { return m_total_size; }
};

}
