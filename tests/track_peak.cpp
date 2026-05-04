#include "oo_alloc/TrackingAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::PoolAllocator pool(32, 8, 10);
  oo_alloc::TrackingAllocator track(pool);

  void* p1 = track.alloc(32, 8);
  void* p2 = track.alloc(32, 8);
  assert(track.curr_bytes() == 64 && "Curr bytes wrong");
  assert(track.active_allocs() == 2 && "Active count wrong");
  assert(track.peak_bytes() == 64 && "Peak bytes wrong");

  track.free(p1);
  assert(track.curr_bytes() == 32 && "Curr bytes wrong post free()");
  assert(track.active_allocs() == 1 && "Active count wrong post free()");
  assert(track.peak_bytes() == 64 && "Peak bytes wrong post free()");
}
