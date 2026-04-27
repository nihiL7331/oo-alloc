#include "oo_alloc/FreeListAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::FreeListAllocator free;
  bool succ = free.init(2048);
  assert(succ && "Failed to initialize");

  void* ptr_1 = free.alloc(128, 8);
  assert(ptr_1 != nullptr && "Failed to allocate ptr_1");

  void* ptr_2 = free.alloc(256, 8);
  assert(ptr_2 != nullptr && "Failed to allocate ptr_2");

  void* ptr_3 = free.alloc(512, 8);
  assert(ptr_3 != nullptr && "Failed to allocate ptr_3");

  free.free(ptr_2);
  free.free(ptr_3);
  free.free(ptr_1);

  void* ptr_4 = free.alloc(1024, 8);
  assert(ptr_4 != nullptr && "Failed to recover memory");
}
