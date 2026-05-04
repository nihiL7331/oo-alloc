#include "oo_alloc/FreeListAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::FreeListAllocator free(2048);

  void* ptr_1 = free.alloc_raw(128, 8);
  assert(ptr_1 != nullptr && "Failed to allocate ptr_1");

  void* ptr_2 = free.alloc_raw(256, 8);
  assert(ptr_2 != nullptr && "Failed to allocate ptr_2");

  void* ptr_3 = free.alloc_raw(512, 8);
  assert(ptr_3 != nullptr && "Failed to allocate ptr_3");

  free.free_raw(ptr_2);
  free.free_raw(ptr_3);
  free.free_raw(ptr_1);

  void* ptr_4 = free.alloc_raw(1024, 8);
  assert(ptr_4 != nullptr && "Failed to recover memory");
}
