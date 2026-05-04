#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <vector>

int main() {
  oo_alloc::BuddyAllocator buddy(1024 * 1024);
  oo_alloc::SlabAllocator slab(&buddy);

  for (int i = 0; i < 100; ++i)
    slab.alloc_raw(16, 8);

  std::vector<void *> ptrs;
  for (int i = 0; i < 5; ++i) {
    void* p = slab.alloc_raw(5000, 8);
    ptrs.push_back(p);
  }

  for (void* ptr : ptrs)
    slab.free_raw(ptr);

  slab.clear();
  
  void* buddy_test = buddy.alloc_raw(512 * 1024, 8);
  assert(buddy_test != nullptr && "Slab clear caused a mem leak in base allocator");
}
