#pragma once
#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

class ArenaAllocator: public IAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_total_size;
  std::size_t m_offset;

public:
  explicit ArenaAllocator(std::size_t size);
  ~ArenaAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free_raw(void* ptr) override; // WARN: noop
  void clear() override;
  std::size_t capacity() const override { return m_total_size; }
};

}
