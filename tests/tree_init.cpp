#include "oo_alloc/FreeTreeAllocator.hpp"
#include <cassert>
#include <cassert>

int main() {
  oo_alloc::FreeTreeAllocator tree;

  bool fail = tree.init(16);
  assert(!fail && "Should reject sizes smaller than min metadata");

  bool succ = tree.init(4096);
  assert(succ && "Failed to initialize");
  assert(tree.capacity() >= 4096 && "Capacity not correctly reported");
}
