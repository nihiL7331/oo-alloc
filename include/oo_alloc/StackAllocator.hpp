#pragma once

#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

class StackAllocator: public IAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_offset;
  std::size_t m_total_size;

public:
  explicit StackAllocator(std::size_t size);
  ~StackAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free_raw(void* ptr) override;
  void clear() override;
  std::size_t capacity() const override { return m_total_size; }
  bool owns(void* ptr) const override;
};

}
