#pragma once
#include "IAllocator.hpp"

namespace oo_alloc {

class ArenaAllocator: public IAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_total_size;
  std::size_t m_offset;

public:
  ArenaAllocator();
  ~ArenaAllocator() override;

  void* alloc(std::size_t size, std::uint8_t align) override;
  void  free(void* ptr) override; // WARN: noop
  bool  init(std::size_t size) override;

  void clear();
};

}
