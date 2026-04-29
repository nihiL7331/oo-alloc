#include "oo_alloc/StackAllocator.hpp"
#include <cassert>
#include <cstddef>

int main() {
  oo_alloc::StackAllocator stack;

  bool succ = stack.init(1024);
  assert(succ && "Failed to initialize");

  void* too_big_ptr = stack.alloc(1024 * 1024, 8);
  assert(too_big_ptr == nullptr && "Allocated out-of-bounds");
}
