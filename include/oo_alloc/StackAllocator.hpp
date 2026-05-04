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

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  void clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_total_size; }
};

}
