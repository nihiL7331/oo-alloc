#include "oo_alloc/BuddyAllocator.hpp"
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
  (void)size;
  assert(false && "TODO");
  return false;
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
