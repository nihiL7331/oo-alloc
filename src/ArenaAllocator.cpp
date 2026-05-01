#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace oo_alloc {

ArenaAllocator::ArenaAllocator()
    : m_start_ptr(nullptr), m_total_size(0), m_offset(0) {}

ArenaAllocator::~ArenaAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

void *ArenaAllocator::alloc(std::size_t size, std::size_t align) {
  std::uintptr_t base_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);
  std::uintptr_t curr_ptr = base_ptr + m_offset;

  std::size_t pad = utils::calc_pad(curr_ptr, align);

  if (m_offset + pad + size > m_total_size)
    return nullptr;

  std::uintptr_t align_ptr = curr_ptr + pad;

  m_offset += (pad + size);

  return reinterpret_cast<void *>(align_ptr);
}

void ArenaAllocator::free(void *ptr) { (void)ptr; }

bool ArenaAllocator::init(std::size_t size) {
  m_total_size = utils::align_up(size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  m_offset = 0;

  return true;
}

void ArenaAllocator::clear() { m_offset = 0; }

void *ArenaAllocator::realloc(void *ptr, std::size_t old_size,
                              std::size_t new_size, std::size_t align) {
  if (new_size <= old_size)
    return ptr;

  std::uint8_t *raw_mem_ptr = static_cast<std::uint8_t *>(ptr);
  std::uint8_t *raw_start_ptr = reinterpret_cast<std::uint8_t *>(m_start_ptr);

  if (raw_start_ptr + m_offset == raw_mem_ptr + old_size) {
    // case 1: realloced elem is the latest one,
    // just resize it
    std::size_t new_offset = m_offset + new_size - old_size;
    if (new_offset > m_total_size) {
      return nullptr;
    }

    m_offset = new_offset;

    return ptr;
  } else {
    // case 2: realloced elem isn't the latest one,
    // need to allocate fresh data after latest

    void *new_ptr = this->alloc(new_size, align);
    if (new_ptr != nullptr)
      std::memcpy(new_ptr, ptr, old_size);

    return new_ptr;
  }
}

} // namespace oo_alloc
