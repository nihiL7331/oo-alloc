#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

namespace oo_alloc {

StackAllocator::StackAllocator(std::size_t size)
  : m_start_ptr(nullptr)
  , m_offset(0)
  , m_total_size(0) {
  if (size == 0)
    return;

  m_total_size = utils::align_up(size, utils::page_size());
  
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    m_total_size = 0;
}

StackAllocator::~StackAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

/* memory structure:
 * [ pad ] [ header ] [ data ]
 *
 * calculates alignment padding,
 * hides the header behind the data.
 * header stores the offset of the stack BEFORE
 * this allocation, allowing for free capabilities
 */
void* StackAllocator::alloc_raw(std::size_t size, std::size_t align) {
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  if (size == 0 || size > SIZE_MAX - align - sizeof(std::size_t))
    return nullptr;

  std::uintptr_t base_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);
  std::uintptr_t curr_ptr = base_ptr + m_offset;

  constexpr std::size_t header_align = alignof(std::size_t);
  constexpr std::size_t header_size = sizeof(std::size_t);

  // ensures that both the data and the header will be aligned
  std::size_t actual_align = std::max(align, header_align);

  std::uintptr_t unalign_ptr = curr_ptr + header_size;
  std::size_t pad = utils::calc_pad(unalign_ptr, actual_align);
  std::uintptr_t data_ptr = unalign_ptr + pad;

  std::size_t alloc_size = header_size + pad + size;
  if (alloc_size > m_total_size - m_offset)
    return nullptr;

  // store the offset in header
  std::size_t *header_ptr = reinterpret_cast<std::size_t *>(data_ptr - header_size);
  *header_ptr = m_offset;

  // push the offset AFTER storing it in the header
  m_offset += alloc_size;

  return reinterpret_cast<void *>(data_ptr);
}

/* rewinds the offset, reading it from the header.
 * because of LIFO it deallocates other data that was
 * allocated after freed data.
 */ 
void StackAllocator::free_raw(void* ptr) {
  if (ptr == nullptr)
    return;

  std::size_t target_offset = *(reinterpret_cast<std::size_t *>(ptr) - 1);
  m_offset = target_offset;
}

/* exactly like in the arena allocator,
 * just move the offset back to 0
 */
void StackAllocator::clear() {
  m_offset = 0;
}

}
