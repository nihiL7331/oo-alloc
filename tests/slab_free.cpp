#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <vector>

int main() {
  oo_alloc::BuddyAllocator buddy(1024 * 1024);
  oo_alloc::SlabAllocator slab(&buddy);

  void* p1 = slab.alloc_raw(32, 8);
  assert(p1 != nullptr && "Alloc failed");
  slab.free_raw(p1);

  void* p2 = slab.alloc_raw(5000, 8);
  assert(p2 != nullptr && "Huge alloc failed");
  slab.free_raw(p2);

  std::vector<void *> ptrs;
  for (int i = 0; i < 63; ++i) {
    void* p = slab.alloc_raw(64, 8);
    if (p)
      ptrs.push_back(p);
  }

  assert(ptrs.size() > 0 && "Failed to allocate multiple blocks");

  for (void* p : ptrs)
    slab.free_raw(p);
}
