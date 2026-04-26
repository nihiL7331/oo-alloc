#include "PoolAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::PoolAllocator pool(sizeof(uint64_t), alignof(uint64_t));

  bool succ = pool.init(32);
  assert(succ && "Failed to initialize");

  void* wrong_size = pool.alloc(sizeof(int), alignof(int));
  assert(wrong_size == nullptr && "Pool should reject wrong sizes");
}
