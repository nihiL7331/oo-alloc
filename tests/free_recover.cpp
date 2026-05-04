#include "oo_alloc/FreeListAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::FreeListAllocator free(2048);

  void* align_ptr = free.alloc_raw(64, 128);
  assert(align_ptr != nullptr && "Failed to allocate 128B align block");

  std::uintptr_t raw_ptr = reinterpret_cast<std::uintptr_t>(align_ptr);
  assert((raw_ptr % 128) == 0 && "Failed to align 128B aligned pointer");

  free.free_raw(align_ptr);

  void* rec_ptr = free.alloc_raw(1024, 8);
  assert(rec_ptr != nullptr && "Failed recover padding on free()");
}
