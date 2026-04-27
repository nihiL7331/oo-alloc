#include "oo_alloc/FreeListAllocator.hpp"
#include <cstdlib>

namespace oo_alloc {

FreeListAllocator::FreeListAllocator()
  : m_start_ptr(nullptr), m_total_size(0), m_free_list_head(nullptr) {}

FreeListAllocator::~FreeListAllocator() {
  if (m_start_ptr != nullptr)
    std::free(m_start_ptr);
}

