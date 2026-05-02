#include "oo_alloc/SlabAllocator.hpp"
#include <cassert>

namespace oo_alloc {

SlabAllocator::SlabAllocator() {

}

SlabAllocator::~SlabAllocator() {

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
      remove_from_list(&(cache.empty_slabs), active_slab);
      push_to_list(&(cache.partial_slabs), active_slab);
    } else {
      // if there are no empty slabs,
      // initialize a new slab
      active_slab = init_slab(cache_idx);
      if (active_slab == nullptr)
        return nullptr;

      // after adding data it becomes a partial slab
      push_to_list(&(cache.partial_slabs), active_slab);
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
    remove_from_list(&(cache.partial_slabs), active_slab);
    push_to_list(&(cache.full_slabs), active_slab);
  }

  return user_ptr;
}

void SlabAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool SlabAllocator::init(std::size_t size) {
  (void)size;
  assert(false && "TODO");
  return false;
}

void SlabAllocator::clear() {
  assert(false && "TODO");
}

void* SlabAllocator::realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

SlabAllocator::SlabHeader* SlabAllocator::init_slab(std::uint8_t cache_idx) noexcept {
  (void)cache_idx;
  assert(false && "TODO");
  return nullptr;
}

}
