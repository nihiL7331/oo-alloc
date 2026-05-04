#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <vector>

int main() {
  oo_alloc::BuddyAllocator buddy(4 * 1024 * 1024);

  std::vector<void*> ptrs;
  for (int i = 0; i < 1024; ++i) {
      void* ptr = buddy.alloc(1024, 8); 
      assert(ptr != nullptr && "Failed to fragment heap");
      ptrs.push_back(ptr);
  }

  for (auto it = ptrs.rbegin(); it != ptrs.rend(); ++it) {
      buddy.free(*it);
  }

  void* giant_ptr = buddy.alloc((4 * 1024 * 1024) - 64, 8);
  assert(giant_ptr != nullptr && "Failed to fully coalesce to max order");
}
