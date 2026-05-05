#include "oo_alloc/SegregatedAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::SegregatedAllocator seg(1024 * 1024);

  void* left   = seg.alloc_raw(100, 8);
  void* middle = seg.alloc_raw(100, 8);
  void* right  = seg.alloc_raw(100, 8);

  assert(left != nullptr && middle != nullptr && right != nullptr && "Failed to setup blocks");

  seg.free_raw(left);
  seg.free_raw(right);

  seg.free_raw(middle);

  void* big_block = seg.alloc_raw(200, 8);
  assert(big_block != nullptr && "Coalescing failed, allocator thinks it's out of large blocks");

  std::uintptr_t old_left_addr = reinterpret_cast<std::uintptr_t>(left);
  std::uintptr_t new_big_addr = reinterpret_cast<std::uintptr_t>(big_block);

  assert(new_big_addr == old_left_addr && "Coalesced block did not reuse the exact memory span");

  seg.free_raw(big_block);
}
