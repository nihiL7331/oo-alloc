#include "oo_alloc/TrackingAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::PoolAllocator pool(32, 8);
  oo_alloc::TrackingAllocator track(pool);

  bool succ = track.init(320);
  assert(succ && "Failed to pass init to pool");

  track.alloc(32, 8);
  track.alloc(32, 8);
  track.alloc(32, 8);
  track.alloc(32, 8);

  track.clear();
  assert(track.curr_bytes() == 0 && "Curr bytes wrong post clear()");
  assert(track.active_allocs() == 0 && "Active count wrong post clear()");
  assert(track.peak_bytes() == 128 && "Peak bytes wrong post clear()");
}
