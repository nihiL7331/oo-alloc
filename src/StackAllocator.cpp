#include "StackAllocator.hpp"
#include <cstdlib>

namespace oo_alloc {

StackAllocator::StackAllocator()
  : m_start_ptr(nullptr), m_offset(0), m_total_size(0) {}

StackAllocator::~StackAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}
