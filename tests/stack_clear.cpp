#include "oo_alloc/StackAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::StackAllocator stack(1024);

  void* ptr = stack.alloc(32, 16);
  stack.alloc(12, 4);
  stack.alloc(64, 8);

  stack.clear();

  void* new_ptr = stack.alloc(64, 16);
  assert(ptr == new_ptr && "clear didn't set m_offset to 0");
}
