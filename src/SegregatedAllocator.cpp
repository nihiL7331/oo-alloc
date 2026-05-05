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
  for (std::uint8_t i = 0; i < NUM_BUCKETS; ++i)
    m_buckets[i] = nullptr;

  FreeBlock* init_block = static_cast<FreeBlock*>(m_start_ptr);

  init_block->header.size = m_total_size;
  init_block->next = nullptr;

  std::uint8_t target_bucket = size_to_bucket(m_total_size);
  m_buckets[target_bucket] = init_block;
}

bool SegregatedAllocator::owns(void* ptr) const {
  std::uintptr_t cast_ptr = reinterpret_cast<std::uintptr_t>(ptr);
  std::uintptr_t start_ptr = reinterpret_cast<std::uintptr_t>(m_start_ptr);

  return cast_ptr >= start_ptr && cast_ptr < start_ptr + m_total_size;
}

void SegregatedAllocator::split_block(std::uint8_t bucket) noexcept {
  // pop the top block from target bucket
  FreeBlock* block = m_buckets[bucket];
  m_buckets[bucket] = block->next;
  if (m_buckets[bucket] != nullptr)
    m_buckets[bucket]->prev = nullptr;

  // calculate orders and sizes
  std::uint8_t order = bucket + MIN_BUCKET_ORDER;
  std::uint8_t new_order = order - 1;
  std::uint8_t new_bucket = bucket - 1;
  std::size_t half_size = 1ULL << new_order;

  // find physical start addresses
  // 'block' points to the struct, which is after the header
  std::uintptr_t left_start = reinterpret_cast<std::uintptr_t>(block) - sizeof(AllocHeader);
  std::uintptr_t right_start = left_start + half_size;

  // update left half metadata
  AllocHeader* left_header = reinterpret_cast<AllocHeader *>(left_start);
  left_header->order = new_order;

  AllocFooter* left_footer = reinterpret_cast<AllocFooter *>(right_start - sizeof(AllocFooter));
  left_footer->order = new_order;

  FreeBlock* left_block = reinterpret_cast<FreeBlock *>(left_start + sizeof(AllocHeader));

  // update right half metadata
  AllocHeader* right_header = reinterpret_cast<AllocHeader *>(right_start);
  right_header->order = new_order;
  right_header->is_prev_free = true;

  AllocFooter* right_footer = reinterpret_cast<AllocFooter *>(left_start + (2 * half_size) - sizeof(AllocFooter));
  right_footer->order = new_order;

  FreeBlock* right_block = reinterpret_cast<FreeBlock *>(right_start + sizeof(AllocHeader));

  // push both halves
  right_block->prev = nullptr;
  right_block->next = m_buckets[new_bucket];
  if (m_buckets[new_bucket] != nullptr)
    m_buckets[new_bucket]->prev = right_block;

  left_block->prev = nullptr;
  left_block->next = right_block;
  right_block->prev = left_block;
  m_buckets[new_bucket] = left_block;
}

}
