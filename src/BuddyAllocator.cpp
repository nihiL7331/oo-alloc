#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <bit>
#include <cassert>

namespace oo_alloc {

void BuddyAllocator::split_block(std::size_t order) noexcept {
  (void)order;
  assert(false && "TODO");
}

void* BuddyAllocator::alloc(std::size_t size, std::size_t align) {
  (void)size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

void BuddyAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool BuddyAllocator::init(std::size_t size) {
  if (size < MIN_BLOCK_SIZE)
    return false;

  std::size_t target_size = std::bit_floor(size);

  m_total_size = utils::align_up(target_size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (!m_start_ptr)
    return false;

  m_free_lists.fill(nullptr);

  std::size_t max_order = size_to_order(m_total_size);

  FreeBlock* parent_block = static_cast<FreeBlock *>(m_start_ptr);
  parent_block->header.set_order(max_order);
  parent_block->header.set_free(true);

  parent_block->prev = nullptr;
  parent_block->next = nullptr;
  m_free_lists[max_order] = parent_block;

  return true;
}

void BuddyAllocator::clear() {
  assert(false && "TODO");
}

void* BuddyAllocator::realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

}
