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
  TrackingAllocator(IAllocator& base_allocator)
    : m_base_allocator(base_allocator) {}
  ~TrackingAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_base_allocator.capacity(); }

  std::size_t get_curr_bytes() const { return m_curr_alloced_bytes; }
  std::size_t get_peak_bytes() const { return m_peak_alloced_bytes; }
  std::size_t get_active_allocs_cnt() const { return m_active_allocs.size(); }
};

}
