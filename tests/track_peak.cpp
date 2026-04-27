#include "TrackingAllocator.hpp"
#include "PoolAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::PoolAllocator pool(32, 8);
  oo_alloc::TrackingAllocator track(pool);

  bool succ = track.init(320);
  assert(succ && "Failed to pass init to pool");

  void* p1 = track.alloc(32, 8);
  void* p2 = track.alloc(32, 8);
  assert(track.get_curr_bytes() == 64 && "Curr bytes wrong");
  assert(track.get_active_allocs_cnt() == 2 && "Active count wrong");
  assert(track.get_peak_bytes() == 64 && "Peak bytes wrong");

  track.free(p1);
  assert(track.get_curr_bytes() == 32 && "Curr bytes wrong post free()");
  assert(track.get_active_allocs_cnt() == 1 && "Active count wrong post free()");
  assert(track.get_peak_bytes() == 64 && "Peak bytes wrong post free()");
}
