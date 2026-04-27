#include "TrackingAllocator.hpp"
#include <iostream>

namespace oo_alloc {

TrackingAllocator::~TrackingAllocator() {
  if (!m_active_allocs.empty())
    std::cerr << "MEM LEAK: "
              << m_active_allocs.size()
              << " allocs were not freed. "
              << "(" << m_curr_alloced_bytes << "B)"
              << std::endl;
}

bool TrackingAllocator::init(std::size_t size) {
  return m_base_allocator.init(size);
}

