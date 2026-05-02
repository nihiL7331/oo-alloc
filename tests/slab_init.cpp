#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include <cassert>

int main() {
  oo_alloc::BuddyAllocator buddy;
  oo_alloc::SlabAllocator slab(&buddy);

  bool succ = slab.init(oo_alloc::utils::page_size());
  assert(succ && "Failed to initialize");

  slab.init(20000);
  assert(slab.capacity() == 0 && "Total size should be 0 before first alloc");
}
