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

  seg.free_raw(big_block);
}
