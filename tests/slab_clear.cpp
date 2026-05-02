#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <vector>

int main() {
  oo_alloc::BuddyAllocator buddy;
  buddy.init(1024 * 1024);

  oo_alloc::SlabAllocator slab(&buddy);
  slab.init(4096);

  for (int i = 0; i < 100; ++i)
    slab.alloc(16, 8);

  std::vector<void *> ptrs;
  for (int i = 0; i < 5; ++i) {
    void* p = slab.alloc(5000, 8);
    ptrs.push_back(p);
  }

  for (void* ptr : ptrs)
    slab.free(ptr);

  slab.clear();
  
  void* buddy_test = buddy.alloc(512 * 1024, 8);
  assert(buddy_test != nullptr && "Slab clear caused a mem leak in base allocator");
}
