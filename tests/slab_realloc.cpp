#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <cstring>

int main() {
  oo_alloc::BuddyAllocator buddy;
  buddy.init(1024 * 1024);
  oo_alloc::SlabAllocator slab(&buddy);
  slab.init(4096);

  void* p1 = slab.alloc(20, 8);
  std::memset(p1, 0xAA, 20);
  
  void* p2 = slab.realloc(p1, 20, 25, 8);
  assert(p1 == p2 && "Realloc moved memory when the cache class was identical");
  assert(static_cast<std::uint8_t*>(p2)[19] == 0xAA && "Data corrupted in same cache realloc");

  void* p3 = slab.alloc(30, 8);
  std::memset(p3, 0xBB, 30);
  
  void* p4 = slab.realloc(p3, 30, 100, 8);
  assert(p3 != p4 && "Realloc failed to move pointer when changing size classes");
  assert(static_cast<std::uint8_t*>(p4)[29] == 0xBB && "Data corrupted during cache jump copy");

  void* p5 = slab.alloc(100, 8);
  std::memset(p5, 0xCC, 100);
  
  void* p6 = slab.realloc(p5, 100, 5000, 8);
  assert(p5 != p6 && "Realloc failed to route to base allocator for huge size");
  assert(static_cast<std::uint8_t*>(p6)[99] == 0xCC && "Data corrupted crossing slab to base boundary");

  void* p7 = slab.realloc(p6, 5000, 64, 8);
  assert(p6 != p7 && "Realloc failed to route back to slab for small size");
  assert(static_cast<std::uint8_t*>(p7)[63] == 0xCC && "Data corrupted crossing base to slab boundary");

  slab.free(p2);
  slab.free(p4);
  slab.free(p7);
}
