#include "TrackingAllocator.hpp"
#include "PoolAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::PoolAllocator pool(32, 8);
  oo_alloc::TrackingAllocator track(pool);

  bool succ = track.init(320);
  assert(succ && "Failed to pass init to pool");

  void* p1 = track.alloc(32, 8);
  assert(p1 != nullptr && "Failed to return mem from base");
  assert(track.get_curr_bytes() == 32 && "Curr bytes wrong");
  assert(track.get_active_allocs_cnt() == 1 && "Active count wrong");
  assert(track.get_peak_bytes() == 32 && "Peak bytes wrong");
}
