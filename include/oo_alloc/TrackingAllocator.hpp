#pragma once

#include "oo_alloc/IAllocator.hpp"
#include <unordered_map>

namespace oo_alloc {

class TrackingAllocator: public IAllocator {
private:
  IAllocator& m_base_allocator;

  std::unordered_map<void *, std::size_t> m_active_allocs;
  std::size_t m_curr_alloced_bytes = 0;
  std::size_t m_peak_alloced_bytes = 0;

public:
  explicit TrackingAllocator(IAllocator& base_allocator)
    : m_base_allocator(base_allocator) {}
  ~TrackingAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free_raw(void* ptr) override;
  void clear() override;
  std::size_t capacity() const override { return m_base_allocator.capacity(); }
  bool owns(void* ptr) const override { return m_base_allocator.owns(ptr); }

  std::size_t curr_bytes() const { return m_curr_alloced_bytes; }
  std::size_t peak_bytes() const { return m_peak_alloced_bytes; }
  std::size_t active_allocs() const { return m_active_allocs.size(); }
};

}
