#include "oo_alloc/TrackingAllocator.hpp"
#include <cassert>
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

bool TrackingAllocator::init(std::size_t size) {
  return m_base_allocator.init(size);
}

void TrackingAllocator::clear() {
  m_curr_alloced_bytes = 0;
  m_active_allocs.clear();
  m_base_allocator.clear();
}

void *TrackingAllocator::realloc(void *ptr, std::size_t old_size,
                              std::size_t new_size, std::size_t align) {
  if (ptr == nullptr)
    return alloc(new_size, align);

  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

  // find the old allocation
  auto got = m_active_allocs.find(ptr);
  if (got == m_active_allocs.end())
    return nullptr;

  // store its size
  std::size_t tracked_old_size = got->second;

  // remove it from the data temporarily
  m_active_allocs.erase(got);
  m_curr_alloced_bytes -= tracked_old_size;

  // call the base allocator
  void* new_ptr = m_base_allocator.realloc(ptr, old_size, new_size, align);

  if (new_ptr != nullptr) {
    // if successed, just track the new pointer.
    // it might be the same address, or it moved elsewhere.
    m_active_allocs[new_ptr] = new_size;
    m_curr_alloced_bytes += new_size;
    
    if (m_curr_alloced_bytes > m_peak_alloced_bytes)
      m_peak_alloced_bytes = m_curr_alloced_bytes;
  } else {
    // if it failed, then most likely the old data
    // is still okay so revert the changes
    m_active_allocs[ptr] = tracked_old_size;
    m_curr_alloced_bytes += tracked_old_size;
  }

  return new_ptr;
}

}
