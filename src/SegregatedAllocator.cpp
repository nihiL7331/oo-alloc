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

/* on alloc_raw, find the smallest bucket that fits
 * the requested size. if the bucket is empty,
 * split bigger-sized buckets until 
 * the wanted bucket size is nonempty.
 * if there's no bigger sizes, then return nullptr
 */
void* SegregatedAllocator::alloc_raw(std::size_t size, std::size_t align) {
  // alignment must be a power of two
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  // ensure there's no integer overflow
  if (size == 0 || size > SIZE_MAX - align - sizeof(AllocHeader))
    return nullptr;

  constexpr std::size_t MAX_BUCKET_SIZE = 1ULL << (MIN_BUCKET_ORDER + NUM_BUCKETS - 1);
  constexpr std::size_t header_size = sizeof(AllocHeader);

  if (size > MAX_BUCKET_SIZE - header_size || align > MAX_BUCKET_SIZE - header_size)
    return nullptr;

  // worst case size
  // 1 byte header + worst case alignment + requested size
  std::size_t actual_size = header_size + (align - 1) + size;

  std::uint8_t bucket_idx = size_to_bucket(actual_size);
  FreeBlock* head = m_buckets[bucket_idx];

  // if the bucket is empty,
  // find a larger block and split it down
  if (head == nullptr) {
    std::uint8_t curr_bucket = bucket_idx;

    // search for bigger block
    while (curr_bucket < NUM_BUCKETS && m_buckets[curr_bucket] == nullptr)
      curr_bucket++;

    // if there's none, there's not enough space
    if (curr_bucket == NUM_BUCKETS)
      return nullptr;

    // split it down
    while (curr_bucket != bucket_idx) {
      split_block(curr_bucket);
      curr_bucket--;
    }

    head = m_buckets[bucket_idx];
  } 

  // pop the head
  m_buckets[bucket_idx] = head->next;
  if (m_buckets[bucket_idx] != nullptr)
    m_buckets[bucket_idx]->prev = nullptr;

  // get the start addr of whole block
  std::uintptr_t start_addr = reinterpret_cast<std::uintptr_t>(head) - header_size;

  AllocHeader* header = reinterpret_cast<AllocHeader *>(start_addr);
  std::uint8_t order = header->order;

  // 1 byte for the back pointer
  std::uintptr_t min_data_addr = start_addr + header_size + sizeof(std::uint8_t);

  // get the aligned addr
  std::uintptr_t data_addr = utils::align_up(min_data_addr, align);

  // calculate how far the data needed to be padded to be aligned,
  // drop that value into the back pointer.
  // 'free_raw' relies on reading that byte
  std::uint8_t offset = static_cast<std::uint8_t>(data_addr - start_addr);
  std::uint8_t* back_ptr = reinterpret_cast<std::uint8_t *>(data_addr - 1);
  *back_ptr = offset;

  // update the right neighbor
  std::size_t block_size = 1ULL << order;
  std::uintptr_t right_neighbor = start_addr + block_size;

  std::uintptr_t heap_end = reinterpret_cast<std::uintptr_t>(m_start_ptr) + m_total_size;

  // only update it if the right neighbor exists (is in bounds)
  if (right_neighbor < heap_end) {
    AllocHeader* right_header = reinterpret_cast<AllocHeader *>(right_neighbor);
    right_header->is_prev_free = false;
  }

  return reinterpret_cast<void *>(data_addr);
}

void SegregatedAllocator::free_raw(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

void SegregatedAllocator::clear() {
  // similarly to the slab allocator,
  // first need to clear the buckets
  for (std::uint8_t i = 0; i < NUM_BUCKETS; ++i)
    m_buckets[i] = nullptr;

  // get the bucket order
  std::uint8_t target_bucket = size_to_bucket(m_total_size);
  std::uint8_t order = target_bucket + MIN_BUCKET_ORDER;

  // write the header
  std::uintptr_t start_addr = reinterpret_cast<std::uintptr_t>(m_start_ptr);
  AllocHeader* header = reinterpret_cast<AllocHeader *>(start_addr);
  header->order = order;
  header->is_prev_free = false;

  // but then we initialize a new block taking up the available space
  // immediately after the header
  FreeBlock* init_block = reinterpret_cast<FreeBlock *>(start_addr + sizeof(AllocHeader));
  init_block->prev = nullptr;
  init_block->next = nullptr;

  // write the footer
  std::uintptr_t end_addr = start_addr + m_total_size;
  AllocFooter* footer = reinterpret_cast<AllocFooter *>(end_addr - sizeof(AllocFooter));
  footer->order = order;

  // place it in the according bucket
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
