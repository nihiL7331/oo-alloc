#include "oo_alloc/TrackingAllocator.hpp"
#include <cassert>
#include <iostream>

namespace oo_alloc {

TrackingAllocator::~TrackingAllocator() {
  if (!m_active_allocs.empty())
    std::cerr << "MEM LEAK: "
              << m_active_allocs.size()
              << " allocations were not freed. "
              << "(" << m_curr_alloced_bytes << "B)"
              << std::endl;
}

/* stores the debug data
 * and allocates via its base allocator
 */
void* TrackingAllocator::alloc(std::size_t size, std::size_t align) {
  void* base_alloc_ptr = m_base_allocator.alloc(size, align);
  if (base_alloc_ptr == nullptr)
    return nullptr;

  m_active_allocs[base_alloc_ptr] = size;
  m_curr_alloced_bytes += size;

  if (m_curr_alloced_bytes > m_peak_alloced_bytes)
    m_peak_alloced_bytes = m_curr_alloced_bytes;

  return base_alloc_ptr;
}

/* reverts the data changes done by the allocation
 */
void TrackingAllocator::free(void* ptr) {
  if (ptr == nullptr)
    return;

  // src: https://cplusplus.com/reference/unordered_map/unordered_map/find/
  auto got = m_active_allocs.find(ptr);
  if (got == m_active_allocs.end())
    return;

  std::size_t ptr_size = got->second;
  m_curr_alloced_bytes -= ptr_size;
  m_active_allocs.erase(got);

  m_base_allocator.free(ptr);
}

void TrackingAllocator::clear() {
  m_curr_alloced_bytes = 0;
  m_active_allocs.clear();
  m_base_allocator.clear();
}

}
