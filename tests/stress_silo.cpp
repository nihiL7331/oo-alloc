#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <vector>
#include <random>
#include <cstring>
#include <cassert>
#include <iostream>

bool verify_mem(void* ptr, std::size_t size, uint8_t pattern) {
  std::uint8_t* bytes = static_cast<std::uint8_t *>(ptr);
  for (std::size_t i = 0; i < size; ++i)
    if (bytes[i] != pattern)
      return false;
  return true;
}

template <class Allocator>
void stress_test(Allocator& alloc, std::size_t block_size, std::size_t align, std::size_t iters) {
  std::mt19937 rng(7331);
  std::uniform_int_distribution<int> action_dist(0, 100);

  std::vector<void *> active_allocs;
  active_allocs.reserve(iters);

  constexpr std::uint8_t POISON_BYTE = 0xBB;

  for (std::size_t i = 0; i < iters; ++i) {
    int action = action_dist(rng);

    if (action < 50 || active_allocs.empty()) {
      void* ptr = alloc.alloc_raw(block_size, align);

      if (ptr != nullptr) {
        assert((reinterpret_cast<std::uintptr_t>(ptr) % align) == 0 && "Alignment failed");

        std::memset(ptr, POISON_BYTE, block_size);
        active_allocs.push_back(ptr);
      }
    } else {
      std::uniform_int_distribution<int> idx_dist(0, active_allocs.size() - 1);
      int idx = idx_dist(rng);
      void* ptr = active_allocs[idx];

      if (!verify_mem(ptr, block_size, POISON_BYTE)) {
        std::cerr << "Memory corruption detected at iteration " << i << std::endl;
        std::abort();
      }

      alloc.free_raw(ptr);

      active_allocs[idx] = active_allocs.back();
      active_allocs.pop_back();
    }
  }

  for (void* ptr : active_allocs) {
    if (!verify_mem(ptr, block_size, POISON_BYTE)) {
      std::cerr << "Memory corruption detected during teardown." << std::endl;
      std::abort();
    }
    
    alloc.free_raw(ptr);
  }
}

int main() {
  constexpr std::size_t MB = 1024 * 1024;
  constexpr std::size_t TEST_SIZE = 64;
  constexpr std::size_t TEST_ALIGN = 8;
  constexpr std::size_t ITERS = 1000000;

  oo_alloc::PoolAllocator pool(TEST_SIZE, TEST_ALIGN, MB);
  stress_test(pool, TEST_SIZE, TEST_ALIGN, ITERS);

  oo_alloc::BuddyAllocator buddy(128 * MB);
  oo_alloc::SlabAllocator slab(&buddy);
  stress_test(slab, TEST_SIZE, TEST_ALIGN, ITERS);
}
