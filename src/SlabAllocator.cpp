#include "oo_alloc/SlabAllocator.hpp"
#include <cassert>

namespace oo_alloc {

SlabAllocator::SlabAllocator() {

}

SlabAllocator::~SlabAllocator() {

}

void* SlabAllocator::alloc(std::size_t size, std::size_t align) {
  (void)size; (void)align;
  assert(false && "TODO");
  return nullptr;
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
