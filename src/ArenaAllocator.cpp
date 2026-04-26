#include "ArenaAllocator.hpp"
namespace oo_alloc {

ArenaAllocator::ArenaAllocator()
  : m_start_ptr(nullptr), m_total_size(0), m_offset(0) {}
   
}
