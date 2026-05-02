#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace oo_alloc {

void BuddyAllocator::split_block(std::size_t order) noexcept {
  // grab the top block of 'order'
  FreeBlock* block = m_free_lists[order];

  // pop 'block'
  m_free_lists[order] = block->next;
  if (m_free_lists[order] != nullptr)
    m_free_lists[order]->prev = nullptr;

  // calculate order and size after split
  std::size_t new_order = order - 1;
  std::size_t half_size = MIN_BLOCK_SIZE << new_order;

  // calculate the pointer of right half (the buddy)
  std::uint8_t* raw_buddy_ptr = reinterpret_cast<std::uint8_t *>(block) + half_size;
  FreeBlock* buddy = reinterpret_cast<FreeBlock *>(raw_buddy_ptr);

  // update 'block' metadata
  block->header.set_order(new_order);
  block->header.set_free(true);

  // update 'buddy' metadata
  buddy->header.set_order(new_order);
  buddy->header.set_free(true);

  // push 'buddy' first.
  // 'alloc' grabs head of the list,
  // so pushing 'block' later grants 
  // better spatial memory locality.
  buddy->prev = nullptr;
  buddy->next = m_free_lists[new_order];
  if (m_free_lists[new_order] != nullptr)
    m_free_lists[new_order]->prev = buddy;
  m_free_lists[new_order] = buddy;

  // push 'block'
  block->prev = nullptr;
  block->next = m_free_lists[new_order];
  if (m_free_lists[new_order] != nullptr)
    m_free_lists[new_order]->prev = block;
  m_free_lists[new_order] = block;
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
  if (ptr == nullptr)
    return;

  // to get the free block data, need to go backwards.
  // its essentially reverting the behavior at the end of alloc.
  std::uint8_t* raw_ptr = static_cast<std::uint8_t *>(ptr) - offsetof(FreeBlock, prev);
  FreeBlock* new_free_block = reinterpret_cast<FreeBlock *>(raw_ptr);

  // setting free twice to ensure theres no unexpected behavior,
  // e.g. if the block doesn't get merged.
  new_free_block->header.set_free(true);
  std::size_t order = new_free_block->header.order();

  // look for buddies, merge if possible
  while (order < MAX_ORDER - 1) {
    FreeBlock* buddy = get_buddy(new_free_block, order);

    std::uintptr_t buddy_addr = reinterpret_cast<std::uintptr_t>(buddy);
    
    // edge case for when get_buddy 
    // returns a address outside of heap
    std::uintptr_t data_end = reinterpret_cast<std::uintptr_t>(m_start_ptr) + m_total_size;
    if (buddy_addr >= data_end)
      break;

    // the buddy (obviously) has to be free to merge,
    // also it has to be of the same order, if it's
    // of different order it is split into smaller pieces
    if (!buddy->header.free() || buddy->header.order() != order)
      break;

    // pop buddy
    FreeBlock* prev_buddy = buddy->prev;
    FreeBlock* next_buddy = buddy->next;

    if (prev_buddy != nullptr)
      prev_buddy->next = next_buddy;
    if (next_buddy != nullptr)
      next_buddy->prev = prev_buddy;
    if (m_free_lists[order] == buddy)
      m_free_lists[order] = next_buddy;

    // merged blocks address starts at the lowest memory address
    new_free_block = std::min(new_free_block, buddy);
    order++;
  }

  // update metadata after memory address shift
  new_free_block->header.set_free(true);
  new_free_block->header.set_order(order);

  // push the newly merged block into the list
  if (m_free_lists[order] != nullptr)
    m_free_lists[order]->prev = new_free_block;
  new_free_block->next = m_free_lists[order];
  new_free_block->prev = nullptr;

  m_free_lists[order] = new_free_block;
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

  // clear the state
  this->clear();

  return true;
}

void BuddyAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

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
}

void* BuddyAllocator::realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) {
  if (ptr == nullptr)
    return alloc(new_size, align);

  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

  // read the actual free space from the header:
  // if user allocated e.g. 33B, he has still 31B free he doesn't know of
  std::uint8_t* raw_ptr = static_cast<std::uint8_t *>(ptr) - offsetof(FreeBlock, prev);
  FreeBlock* old_block = reinterpret_cast<FreeBlock *>(raw_ptr);

  std::size_t curr_order = old_block->header.order();
  std::size_t target_order = size_to_order(new_size);

  // if the "hidden" free space was enough to fit
  // 'new_size', just return the pointer
  if (curr_order >= target_order)
    return ptr;

  // standard 'alloc' -> 'copy' -> 'free' cycle,
  // in-place is not worth it for the buddy allocator
  void* new_ptr = alloc(new_size, align);
  if (new_ptr == nullptr)
    return nullptr;

  // copy the data
  std::memcpy(new_ptr, ptr, old_size);

  // free the old block
  free(ptr);
  
  return new_ptr;
}

}
