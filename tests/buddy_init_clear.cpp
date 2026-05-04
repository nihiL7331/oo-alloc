#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::BuddyAllocator buddy(100 * 1024 * 1024);

  std::size_t actual_cap = buddy.capacity();
  assert(actual_cap == (128 * 1024 * 1024) && "Failed to bit_ceil to 128MB");

  void* ptr1 = buddy.alloc_raw(1024, 8);
  assert(ptr1 != nullptr && "Failed basic allocation");

  buddy.clear();

  void* parent_ptr = buddy.alloc_raw(actual_cap - 32, 8);
  assert(parent_ptr != nullptr && "Clear failed to create the parent block");
}
