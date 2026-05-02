#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <vector>

int main() {
  oo_alloc::BuddyAllocator buddy;
  bool succ_b = buddy.init(1024 * 1024);
  assert(succ_b && "Failed to initialize buddy");

  oo_alloc::SlabAllocator slab(&buddy);
  bool succ_s = slab.init(4096);
  assert(succ_s && "Failed to initialize slab");

  void* p1 = slab.alloc(32, 8);
  assert(p1 != nullptr && "Alloc failed");
  slab.free(p1);

  void* p2 = slab.alloc(5000, 8);
  assert(p2 != nullptr && "Huge alloc failed");
  slab.free(p2);

  std::vector<void *> ptrs;
  for (int i = 0; i < 63; ++i) {
    void* p = slab.alloc(64, 8);
    if (p)
      ptrs.push_back(p);
  }

  assert(ptrs.size() > 0 && "Failed to allocate multiple blocks");

  for (void* p : ptrs)
    slab.free(p);
}
