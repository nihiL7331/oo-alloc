#include "oo_alloc/StackAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::StackAllocator stack(1024);

  void* ptr1 = stack.alloc(32, 16);
  assert(ptr1 != nullptr && "Failed to allocate ptr1");
  assert(reinterpret_cast<std::uintptr_t>(ptr1) % 16 == 0 && "Alignment to 16B failed");
}
