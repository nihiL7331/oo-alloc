#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <random>
#include <cstring>
#include <cassert>
#include <iostream>

struct AllocRecord {
  void* ptr;
  std::size_t size;
  std::size_t align;
};

bool verify_mem(void* ptr, std::size_t size, uint8_t pattern) {
  std::uint8_t* bytes = static_cast<std::uint8_t *>(ptr);
  for (std::size_t i = 0; i < size; ++i)
    if (bytes[i] != pattern)
      return false;
  return true;
}

template <class Allocator>
void stress_test(std::size_t heap_size, std::size_t iters) {
  Allocator alloc;
  alloc.init(heap_size);

  std::mt19937 rng(7331);
  std::uniform_int_distribution<int> action_dist(0, 100);
  std::uniform_int_distribution<std::size_t> size_dist(1, 4096);

  std::vector<std::size_t> aligns = {8, 16, 32, 64, 128};
  std::uniform_int_distribution<int> align_dist(0, aligns.size() - 1);

  std::vector<AllocRecord> active_allocs;
  active_allocs.reserve(iters);

  constexpr std::uint8_t POISON_BYTE = 0xAA;

  for (std::size_t i = 0; i < iters; ++i) {
    int action = action_dist(rng);

    if (action < 60 || active_allocs.empty()) {
      std::size_t size = size_dist(rng);
      std::size_t align = aligns[align_dist(rng)];

      void* ptr = alloc.alloc(size, align);

      if (ptr != nullptr) {
        assert((reinterpret_cast<std::uintptr_t>(ptr) % align) == 0 && "Alignment failed");

        std::memset(ptr, POISON_BYTE, size);

        active_allocs.push_back({ptr, size, align});
      }
    } else {
      std::uniform_int_distribution<int> idx_dist(0, active_allocs.size() - 1);
      int idx = idx_dist(rng);
      AllocRecord record = active_allocs[idx];

      if (!verify_mem(record.ptr, record.size, POISON_BYTE)) {
        std::cerr << "Memory corruption detected at iteration " << i << std::endl;
        std::abort();
      }

      alloc.free(record.ptr);

      active_allocs[idx] = active_allocs.back();
      active_allocs.pop_back();
    }
  }

  for (const auto& record : active_allocs) {
    if (!verify_mem(record.ptr, record.size, POISON_BYTE)) {
      std::cerr << "Memory corruption detected during teardown." << std::endl;
      std::abort();
    }

    alloc.free(record.ptr);
  }
}

int main() {
  constexpr std::size_t MB = 1024 * 1024;
  constexpr std::size_t ITERS = 100000;

  stress_test<oo_alloc::FreeListAllocator>(64 * MB, ITERS);
  stress_test<oo_alloc::FreeTreeAllocator>(64 * MB, ITERS);
  stress_test<oo_alloc::BuddyAllocator>(64 * MB, ITERS);
}
