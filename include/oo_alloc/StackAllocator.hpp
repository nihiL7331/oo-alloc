#pragma once
#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

class StackAllocator: public IAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_offset;
  std::size_t m_total_size;

public:
  StackAllocator();
  ~StackAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
};

}
