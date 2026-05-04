#include "oo_alloc/StackAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::StackAllocator stack(1024);

  void* ptr1 = stack.alloc(12, 4);
  void* ptr2 = stack.alloc(64, 8);

  stack.free(ptr2);

  void* ptr3 = stack.alloc(64, 8);
  assert(ptr2 == ptr3 && "Header read/roll back fail");
}
