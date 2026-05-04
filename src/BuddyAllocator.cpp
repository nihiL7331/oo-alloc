#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace oo_alloc {

BuddyAllocator::BuddyAllocator(std::size_t size) 
  : m_start_ptr(nullptr)
  , m_total_size(0) {
  if (size < MIN_BLOCK_SIZE)
    return;

  // force the size to be a power of 2
  std::size_t target_size = std::bit_floor(size);

  // ensure that 'm_total_size' is a multiple of 'page_size'.
  // page sizes themselves are powers of two, 
  // so this keeps the power of 2 rule.
  m_total_size = utils::align_up(target_size, utils::page_size());

  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr) {
    m_total_size = 0;
    return;
  }

  m_free_lists.fill(nullptr);

  // clear the state
  this->clear();
}

BuddyAllocator::~BuddyAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
    m_total_size = 0;
  }
}

void BuddyAllocator::split_block(std::uint8_t order) noexcept {
  // grab the top block of 'order'
  FreeBlock* block = m_free_lists[order];

  // pop 'block'
  m_free_lists[order] = block->next;
  if (m_free_lists[order] != nullptr)
    m_free_lists[order]->prev = nullptr;

  // calculate order and size after split
  std::uint8_t new_order = order - 1;
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
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  if (size == 0 || size > SIZE_MAX - align - sizeof(AllocHeader))
    return nullptr;

  // we need space for data, header, and enough room to shift
  // the data pointer forward until it hits an 'align' boundary.
  std::size_t total_req_size = size + align + sizeof(AllocHeader);
  std::uint8_t target_order = size_to_order(total_req_size);

  // if there's no free block of wanted order,
  // split bigger blocks (of greater order)
  // down to create it.
  if (m_free_lists[target_order] == nullptr) {
    std::uint8_t curr_order = target_order;

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
  
  std::uintptr_t block_start = reinterpret_cast<std::uintptr_t>(used_block);
  std::uintptr_t min_data_ptr = block_start + sizeof(AllocHeader);
  std::uintptr_t data_ptr = utils::align_up(min_data_ptr, align);

  AllocHeader* header = reinterpret_cast<AllocHeader *>(data_ptr - sizeof(AllocHeader));
  header->set_free(false);
  header->set_order(target_order);

  std::size_t offset_size = data_ptr - block_start;
  // if someone asks for >64KB alignment,
  // must refuse it because it can't be stored
  if (offset_size > UINT16_MAX)
    return nullptr;

  header->offset = static_cast<std::uint16_t>(data_ptr - block_start);

  return reinterpret_cast<void *>(data_ptr);
}

void BuddyAllocator::free(void* ptr) {
  if (ptr == nullptr)
    return;

  // header is always one AllocHeader size behind ptr
  AllocHeader* header = static_cast<AllocHeader *>(ptr) - 1;

  std::uintptr_t data_ptr = reinterpret_cast<std::uintptr_t>(ptr);
  std::uintptr_t block_start = data_ptr - header->offset;

  FreeBlock* new_free_block = reinterpret_cast<FreeBlock *>(block_start);

  std::uint8_t order = header->order();
  new_free_block->header.set_free(true);

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

void BuddyAllocator::clear() {
  if (m_start_ptr == nullptr)
    return;

  m_free_lists.fill(nullptr);

  // calculate the order based on the final total size
  // don't use size_to_order here, because don't want header padding here
  std::uint8_t max_order = static_cast<std::uint8_t>(std::countr_zero(m_total_size) - std::countr_zero(MIN_BLOCK_SIZE));

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

  // grab the header by going sizeof(AllocHeader) backwards
  AllocHeader* header = static_cast<AllocHeader *>(ptr) - 1;
  std::uint8_t curr_order = header->order();

  // calculate how many bytes are left in block
  std::size_t block_size = MIN_BLOCK_SIZE << curr_order;
  std::size_t avail_space = block_size - header->offset;

  std::uintptr_t data_addr = reinterpret_cast<std::uintptr_t>(ptr);
  bool aligned = (data_addr % align) == 0;

  // if the "hidden" free space was enough to fit
  // 'new_size', and its aligned, just return the pointer
  if (aligned && new_size <= avail_space)
    return ptr;

  // standard 'alloc' -> 'copy' -> 'free' cycle,
  // in-place is not worth it for the buddy allocator
  void* new_ptr = this->alloc(new_size, align);
  if (new_ptr == nullptr)
    return nullptr;

  // copy the data
  std::size_t copy_size = std::min(old_size, new_size);
  std::memcpy(new_ptr, ptr, copy_size);

  // free the old block
  this->free(ptr);
  
  return new_ptr;
}

}
