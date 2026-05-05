#include "oo_alloc/SegregatedAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::SegregatedAllocator seg(1024 * 1024);

  void* ptr = seg.alloc_raw(100, 32);
  assert(ptr != nullptr && "Failed to return memory");

  std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
  assert((addr % 32) == 0 && "Returned pointer wasn't 32B aligned");

  int* int_arr = static_cast<int *>(ptr);
  for (int i = 0; i < 25; ++i)
    int_arr[i] = i * 4562;

  for (int i = 0; i < 25; ++i)
    assert(int_arr[i] == i * 4562 && "Memory corruption during read");

  assert(seg.owns(ptr) && "Doesn't own its allocated memory");
  int stack_var = 5;
  assert(!seg.owns(&stack_var) && "Claims to own stack memory");

  seg.free_raw(ptr);
}
