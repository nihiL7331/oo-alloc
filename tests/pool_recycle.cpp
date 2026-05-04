#include "oo_alloc/PoolAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::PoolAllocator pool(sizeof(uint64_t), alignof(uint64_t), 4);

  void* p1 = pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));
  pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));
  void* p3 = pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));
  pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));

  pool.free(p3);

  void* recycled_p3 = pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));
  assert(recycled_p3 == p3 && "Did not recycle freed chunk correctly");

  pool.clear();

  void* recycled_p1 = pool.alloc_raw(sizeof(uint64_t), alignof(uint64_t));
  assert(recycled_p1 == p1 && "Clear didn't reset free list to start");
}
