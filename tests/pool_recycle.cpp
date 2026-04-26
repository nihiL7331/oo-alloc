#include "PoolAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::PoolAllocator pool(sizeof(uint64_t), alignof(uint64_t));

  bool succ = pool.init(32);
  assert(succ && "Failed to initialize");

  void* p1 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  void* p3 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  pool.alloc(sizeof(uint64_t), alignof(uint64_t));

  pool.free(p3);

  void* recycled_p3 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  assert(recycled_p3 == p3 && "Did not recycle freed chunk correctly");

  pool.clear();

  void* recycled_p1 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  assert(recycled_p1 == p1 && "Clear didn't reset free list to start");
}
