#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>

int main() {
  oo_alloc::BuddyAllocator buddy(64 * 1024 * 1024);
  oo_alloc::SlabAllocator slab(&buddy);

  void* p1 = slab.alloc(16, 8);
  assert(p1 != nullptr && "Failed standard small allocation");

  void* p2 = slab.alloc(oo_alloc::utils::page_size() / 3, 8);
  assert(p2 != nullptr && "Failed huge allocation routing");

  for (int i = 0; i < 1000; ++i) {
    void* p = slab.alloc(64, 8);
    assert(p != nullptr && "Failed to allocate after multiple slab creations");
  }
}
