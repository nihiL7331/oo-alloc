#include "oo_alloc/FreeTreeAllocator.hpp"
#include <cassert>

namespace oo_alloc {


FreeTreeAllocator::~FreeTreeAllocator() {

}

void* FreeTreeAllocator::alloc(std::size_t size, std::size_t align) {
  (void)size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

void FreeTreeAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool FreeTreeAllocator::init(std::size_t size) {
  (void)size;
  assert(false && "TODO");
  return false;
}

void FreeTreeAllocator::clear() {
  assert(false && "TODO");
}

void* FreeTreeAllocator::realloc(void* ptr, std::size_t old_size, 
              std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

}
