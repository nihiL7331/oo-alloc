#include "oo_alloc/SegregatedAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>

int main() {
  oo_alloc::SegregatedAllocator seg(oo_alloc::utils::page_size());

  void* too_big = seg.alloc_raw(oo_alloc::utils::page_size() * 2, 8);
  assert(too_big == nullptr && "Handed out memory larger than its pool");

  void* chunk1 = seg.alloc_raw(oo_alloc::utils::page_size() / 4, 8);
  void* chunk2 = seg.alloc_raw(oo_alloc::utils::page_size() / 4, 8);
  
  assert(chunk1 != nullptr && "Failed to grab chunk 1");
  assert(chunk2 != nullptr && "Failed to grab chunk 2");

  void* chunk3 = seg.alloc_raw(oo_alloc::utils::page_size() / 2, 8);
  assert(chunk3 == nullptr && "Failed to respect OOM boundary");

  seg.free_raw(chunk1);
  seg.free_raw(chunk2);
}
