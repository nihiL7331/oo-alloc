#include "oo_alloc/FreeTreeAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::FreeTreeAllocator tree(4096);

  void* ptr1 = tree.alloc_raw(128, 8);
  assert(ptr1 != nullptr && "Failed to allocate 128B");
  assert(reinterpret_cast<std::uintptr_t>(ptr1) % 8 == 0 && "Pointer is not 8B aligned");

  void* ptr2 = tree.alloc_raw(64, 1);
  assert(ptr2 != nullptr && "Failed to allocate 64B");
  assert(reinterpret_cast<std::uintptr_t>(ptr2) % alignof(void *) == 0 && "Failed to set min pointer alignment");

  void* ptr_overflow = tree.alloc_raw(tree.capacity() * 2, 8);
  assert(ptr_overflow == nullptr && "Failed to return nullptr on overflow");

  tree.free_raw(ptr1);
  tree.free_raw(ptr2);
}
