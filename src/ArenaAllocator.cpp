#include "ArenaAllocator.hpp"
#include <cstdlib>

namespace oo_alloc {

ArenaAllocator::ArenaAllocator()
  : m_start_ptr(nullptr), m_total_size(0), m_offset(0) {}

ArenaAllocator::~ArenaAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

   
}
