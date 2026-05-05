#include "oo_alloc/SegregatedAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>
#include <cstdint>

namespace oo_alloc {

SegregatedAllocator::SegregatedAllocator(std::size_t size) 
  : m_start_ptr(nullptr)
  , m_total_size(0) {
  if (size == 0)
    return;

  m_total_size = utils::align_up(size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr) {
    m_total_size = 0;
    return;
  }

  this->clear();
}

SegregatedAllocator::~SegregatedAllocator() {

}

void* SegregatedAllocator::alloc_raw(std::size_t size, std::size_t align) {
  (void)size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

void SegregatedAllocator::free_raw(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

void SegregatedAllocator::clear() {
  assert(false && "TODO");
}

bool SegregatedAllocator::owns(void* ptr) const {
  std::uintptr_t cast_ptr = reinterpret_cast<std::uintptr_t>(ptr);
  std::uintptr_t start_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);

  return cast_ptr >= start_ptr && cast_ptr < start_ptr + m_total_size;
}

}
