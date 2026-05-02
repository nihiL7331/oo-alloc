#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <bit>
#include <cassert>
#include <cstdint>

namespace oo_alloc {

void BuddyAllocator::split_block(std::size_t order) noexcept {
  (void)order;
  assert(false && "TODO");
}

void* BuddyAllocator::alloc(std::size_t size, std::size_t align) {
  // calculate order
  // this handles header + min sizes internally,
  // no need for if statements for that here
  std::size_t target_order = size_to_order(size);

  // ensure that block is big enough to naturally align.
  // since each blocks size is a power of 2,
  // if MIN_BLOCK_SIZE << target_order >= align then the data is aligned
  if ((MIN_BLOCK_SIZE << target_order) < align)
    target_order = size_to_order(align - sizeof(AllocHeader));
  
  // if there's no free block of wanted order,
  // split bigger blocks (of greater order)
  // down to create it.
  if (m_free_lists[target_order] == nullptr) {
    std::size_t curr_order = target_order;

    // find the smallest order, that is greater than 'target_order'
    while (curr_order < MAX_ORDER && m_free_lists[curr_order] == nullptr)
      curr_order++;

    // if not found, return (not enough space)
    if (curr_order == MAX_ORDER)
      return nullptr;

    //  split the block down
    while (curr_order != target_order) {
      split_block(curr_order);
      curr_order--;
    }
  }

  // pop the wanted block
  FreeBlock* used_block = m_free_lists[target_order];

  m_free_lists[target_order] = used_block->next;
  if (m_free_lists[target_order] != nullptr)
    m_free_lists[target_order]->prev = nullptr;

  used_block->header.set_free(false);

  // because of the 'FreeBlock' layout,
  // user data begins where the 'prev' pointer
  // sits when the block is free.
  std::uintptr_t data_ptr = reinterpret_cast<std::uintptr_t>(used_block) + offsetof(FreeBlock, prev);
  return reinterpret_cast<void *>(data_ptr);
}

void BuddyAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool BuddyAllocator::init(std::size_t size) {
  if (size < MIN_BLOCK_SIZE)
    return false;

  // force the size to be a power of 2
  std::size_t target_size = std::bit_floor(size);

  // ensure that 'm_total_size' is a multiple of 'page_size'.
  // page sizes themselves are powers of two, 
  // so this keeps the power of 2 rule.
  m_total_size = utils::align_up(target_size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (!m_start_ptr)
    return false;

  m_free_lists.fill(nullptr);

  // calculate the order based on the final total size
  std::size_t max_order = size_to_order(m_total_size);

  // create the biggest, parent block that later will be split
  // for allocations
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
