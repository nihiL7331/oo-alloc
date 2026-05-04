#include "oo_alloc/PoolAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::PoolAllocator pool(sizeof(uint64_t), alignof(uint64_t), 4);

  void* p1 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  void* p2 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  void* p3 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  void* p4 = pool.alloc(sizeof(uint64_t), alignof(uint64_t));
  assert(p1 && p2 && p3 && p4 && "Failed to allocate valid chunks");

  void* p5 = pool.alloc(1024 * 1024, alignof(uint64_t));
  assert(p5 == nullptr && "Did not return nullptr when exhausted");
}
