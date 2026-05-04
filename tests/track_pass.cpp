#include "oo_alloc/TrackingAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::PoolAllocator pool(32, 8, 10);
  oo_alloc::TrackingAllocator track(pool);

  void* p1 = track.alloc_raw(32, 8);
  assert(p1 != nullptr && "Failed to return mem from base");
  assert(track.curr_bytes() == 32 && "Curr bytes wrong");
  assert(track.active_allocs() == 1 && "Active count wrong");
  assert(track.peak_bytes() == 32 && "Peak bytes wrong");
}
