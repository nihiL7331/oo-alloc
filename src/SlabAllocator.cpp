#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/IAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace oo_alloc {

SlabAllocator::SlabAllocator(IAllocator* base_allocator) 
  : m_page_size(utils::page_size())
  , m_base_allocator(base_allocator) {
  // initialize the caches with powers of 2,
  // starting at '1 << MIN_CACHE_ORDER'
  std::size_t curr_size = 1 << MIN_CACHE_ORDER;

  for (std::uint8_t i = 0; i < NUM_CACHES; ++i) {
    m_caches[i].init(curr_size);
    curr_size <<= 1;
  }
}

SlabAllocator::~SlabAllocator() {
  this->clear();
}

void* SlabAllocator::alloc(std::size_t size, std::size_t align) {
  if (align == 0 || (align & (align - 1)) != 0)
    return nullptr;

  if (size == 0 || size > SIZE_MAX - align)
    return nullptr;

  // calculate maximum size that can be stored in cache
  constexpr std::size_t MAX_CACHE_SIZE = 1 << (MIN_CACHE_ORDER + NUM_CACHES - 1);

  // if the allocated space is larger, then route to base
  // if the alignment is larger, then it also needs to be routed
  // because it can't fit after alignment
  if (size > MAX_CACHE_SIZE || align > MAX_CACHE_SIZE)
    return m_base_allocator->alloc(size, align);

  // ensure the cache picked is large enough
  // to fit the aligned size
  std::size_t actual_size = std::max(size, align);

  // find the correct CacheManager
  std::uint8_t cache_idx = size_to_cache_idx(actual_size);
  CacheManager& cache = m_caches[cache_idx];

  // find a slab with free space, starting with
  // the partial slabs
  SlabHeader* active_slab = cache.partial_slabs;

  if (active_slab == nullptr) {
    // if there are no partial slabs,
    // look in the empty slabs
    active_slab = cache.empty_slabs;

    if (active_slab != nullptr) {
      // if there is a empty slab,
      // move it to partial
      remove_from_list(cache.empty_slabs, active_slab);
      push_to_list(cache.partial_slabs, active_slab);
    } else {
      // if there are no empty slabs,
      // initialize a new slab
      active_slab = init_slab(cache_idx);
      if (active_slab == nullptr)
        return nullptr;

      // after adding data it becomes a partial slab
      push_to_list(cache.partial_slabs, active_slab);
    }
  }

  // take the first free slot from the intrusive list
  void* user_ptr = active_slab->free_list_head;

  // read the pointer inside the free slot to find the next free slot
  void** next_ptr = static_cast<void **>(user_ptr);
  active_slab->free_list_head = *next_ptr;

  active_slab->used++;

  // if the slab just become full, move it to full slabs
  if (active_slab->used == active_slab->capacity) {
    remove_from_list(cache.partial_slabs, active_slab);
    push_to_list(cache.full_slabs, active_slab);
  }

  return user_ptr;
}

void SlabAllocator::free(void* ptr) {
  if (ptr == nullptr)
    return;

  // get the header by masking ptr
  std::uintptr_t raw_header_ptr = reinterpret_cast<std::uintptr_t>(ptr) & ~(m_page_size - 1);
  SlabHeader* header_ptr = reinterpret_cast<SlabHeader *>(raw_header_ptr);

  if (header_ptr->id != SLAB_ID) {
    // if the 'id' isnt 0x51AB, it means that
    // this data haven't went through the 'init_slab'
    // code, hence it was allocated by the 'm_base_allocator'
    m_base_allocator->free(ptr);
    return;
  }

  // push 'ptr' onto the intrusive free list
  void** list_ptr = static_cast<void **>(ptr);
  *list_ptr = header_ptr->free_list_head;
  header_ptr->free_list_head = ptr;
  header_ptr->used--;

  CacheManager& cache = m_caches[header_ptr->cache_idx];

  // if slab is almost full, it means that it was full before,
  // so it needs to be moved to partial
  if (header_ptr->used == header_ptr->capacity - 1) {
    remove_from_list(cache.full_slabs, header_ptr);
    push_to_list(cache.partial_slabs, header_ptr);
  }

  // not using 'else if' for edge case, where
  // capacity is 1, making each slab have 2 states:
  // full and empty.
  if (header_ptr->used == 0) {
    remove_from_list(cache.partial_slabs, header_ptr);
    push_to_list(cache.empty_slabs, header_ptr);
  }
}

void SlabAllocator::clear() {
  for (std::uint8_t i = 0; i < NUM_CACHES; ++i) {
    CacheManager& cache = m_caches[i];
    clear_slab_list(cache.full_slabs);
    clear_slab_list(cache.partial_slabs);
    clear_slab_list(cache.empty_slabs);
  }
}

void* SlabAllocator::realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) {
  if (ptr == nullptr)
    return alloc(new_size, align);

  if (new_size == 0) {
    this->free(ptr);
    return nullptr;
  }

  constexpr std::size_t MAX_CACHE_SIZE = 1 << (MIN_CACHE_ORDER + NUM_CACHES - 1);
  bool old_is_big = old_size > MAX_CACHE_SIZE;
  bool new_is_big = new_size > MAX_CACHE_SIZE;

  if (old_is_big && new_is_big)
    return m_base_allocator->realloc(ptr, old_size, new_size, align);

  if (old_is_big != new_is_big) {
    void* new_ptr = this->alloc(new_size, align);

    if (new_ptr != nullptr) {
      std::size_t copy_size = std::min(old_size, new_size);
      std::memcpy(new_ptr, ptr, copy_size);

      m_base_allocator->free(ptr);
    }
    return new_ptr;
  }

  std::uint8_t old_idx = size_to_cache_idx(old_size);
  std::uint8_t new_idx = size_to_cache_idx(new_size);

  if (old_idx == new_idx)
    return ptr;

  void* new_ptr = alloc(new_size, align);
  if (new_ptr != nullptr) {
    std::size_t copy_size = std::min(old_size, new_size);
    std::memcpy(new_ptr, ptr, copy_size);
    this->free(ptr);
  }

  return new_ptr;
}

SlabAllocator::SlabHeader* SlabAllocator::init_slab(std::uint8_t cache_idx) noexcept {
  // get 'object_size' from cache for later calculations
  std::size_t object_size = m_caches[cache_idx].object_size;

  // allocate the raw page from 'm_base_allocator'
  void* ptr = m_base_allocator->alloc(m_page_size, m_page_size);
  if (ptr == nullptr)
    return nullptr;

  // initialize the header, it's positioned directly
  // at the raw page pointer position
  SlabHeader* header_ptr = static_cast<SlabHeader *>(ptr);
  header_ptr->prev = nullptr;
  header_ptr->next = nullptr;
  header_ptr->used = 0;
  // each page is split up to equally sized objects
  header_ptr->capacity = static_cast<std::uint16_t>((m_page_size - sizeof(SlabHeader)) / object_size);
  header_ptr->cache_idx = cache_idx;
  // this is used to differ base allocator allocated
  // data from slabs
  header_ptr->id = SLAB_ID;

  // get the beginning of actual data/payload 
  // which is directly after the header
  void* free_ptr = reinterpret_cast<void *>(header_ptr + 1);
  header_ptr->free_list_head = free_ptr;

  // iterate through the capacity, creating an 
  // intrusive singly-linked list
  for (std::uint16_t i = 0; i < header_ptr->capacity - 1; ++i) {
    void** cast_free_ptr = static_cast<void **>(free_ptr);

    // each object is 'object_size' away from previous object
    void* next_free_ptr = static_cast<std::uint8_t *>(free_ptr) + object_size;
    *cast_free_ptr = next_free_ptr;
    free_ptr = next_free_ptr;
  }

  // mark the last object's 'next' as nullptr
  void** cast_free_ptr = static_cast<void **>(free_ptr);
  *cast_free_ptr = nullptr;

  return header_ptr;
}

}
