#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::BuddyAllocator buddy;
  bool succ = buddy.init(1024 * 1024);
  assert(succ && "Failed to initialize");

  void* small_ptr = buddy.alloc(4, 8);
  assert(small_ptr != nullptr && "Failed small allocation");

  void* align_ptr = buddy.alloc(8, 64);
  assert(align_ptr != nullptr && "Failed aligned allocation");
  std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(align_ptr);
  assert((addr % 64 == 0) && "Failed to naturally align block");

  int alloc_cnt = 0;
  while (buddy.alloc(32, 8) != nullptr) {
      alloc_cnt++;
  }
  
  assert(alloc_cnt > 0 && "Did not allocate any exhaustion blocks");
  void* overflow_ptr = buddy.alloc(32, 8);
  assert(overflow_ptr == nullptr && "Did not return nullptr when overflowed");
}
