#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/IAllocator.hpp"
#include <cassert>

namespace oo_alloc {

SlabAllocator::SlabAllocator(IAllocator* base_allocator) 
  : m_total_size(0), m_page_size(0), m_base_allocator(base_allocator) {
  // initialize the caches with powers of 2,
  // starting at '1 << MIN_CACHE_ORDER'
  std::size_t curr_size = 1 << MIN_CACHE_ORDER;

  for (std::uint8_t i = 0; i < NUM_CACHES; ++i) {
    m_caches[i].init(curr_size);
    curr_size <<= 1;
  }
}

SlabAllocator::~SlabAllocator() {
  clear();
}

void* SlabAllocator::alloc(std::size_t size, std::size_t align) {
  // if the request is larger than half a page,
  // it's too big for caches, so it needs to be
  // allocated via the base allocator
  if (size > m_page_size / 2)
    return m_base_allocator->alloc(size, align);

  // find the correct CacheManager
  std::uint8_t cache_idx = size_to_cache_idx(size);
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

bool SlabAllocator::init(std::size_t size) {
  (void)size;
  assert(false && "TODO");
  return false;
}

void SlabAllocator::clear() {
  for (std::uint8_t i = 0; i < NUM_CACHES; ++i) {
    CacheManager& cache = m_caches[i];
    clear_slab_list(cache.full_slabs);
    clear_slab_list(cache.partial_slabs);
    clear_slab_list(cache.empty_slabs);
  }

  m_total_size = 0;
}

void* SlabAllocator::realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
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
  header_ptr->capacity = (m_page_size - sizeof(SlabHeader)) / object_size;
  header_ptr->cache_idx = cache_idx;

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
