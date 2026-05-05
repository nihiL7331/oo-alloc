#include "oo_alloc/SegregatedAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::SegregatedAllocator seg(64 * 1024);

  void* ptr1 = seg.alloc_raw(8, 8);
  void* ptr2 = seg.alloc_raw(8, 8); 

  assert(ptr1 != nullptr && "Failed to split block 1");
  assert(ptr2 != nullptr && "Failed to split block 2");

  std::uintptr_t addr1 = reinterpret_cast<std::uintptr_t>(ptr1);
  std::uintptr_t addr2 = reinterpret_cast<std::uintptr_t>(ptr2);

  std::uint8_t offset1 = *reinterpret_cast<std::uint8_t*>(addr1 - 1);
  std::uint8_t offset2 = *reinterpret_cast<std::uint8_t*>(addr2 - 1);
  
  std::uintptr_t phys_start1 = addr1 - offset1;
  std::uintptr_t phys_start2 = addr2 - offset2;

  assert((phys_start2 - phys_start1) == 32 && "Buddy blocks are not adjacent");

  seg.free_raw(ptr1);
  seg.free_raw(ptr2);
}
