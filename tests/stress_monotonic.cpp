#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
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
void stress_test_stack(std::size_t heap_size, std::size_t iters) {
  Allocator alloc(heap_size);

  std::mt19937 rng(7331);
  std::uniform_int_distribution<int> action_dist(0, 100);
  std::uniform_int_distribution<std::size_t> size_dist(1, 4096);
  std::vector<std::size_t> aligns = {8, 16, 32, 64};
  std::uniform_int_distribution<int> align_dist(0, aligns.size() - 1);

  std::vector<AllocRecord> active_allocs;
  constexpr std::uint8_t POISON_BYTE = 0xCC;

  for (std::size_t i = 0; i < iters; ++i) {
    int action = action_dist(rng);

    if (action < 60 || active_allocs.empty()) {
      std::size_t size = size_dist(rng);
      std::size_t align = aligns[align_dist(rng)];

      void* ptr = alloc.alloc_raw(size, align);

      if (ptr) {
        assert((reinterpret_cast<std::uintptr_t>(ptr) % align) == 0);
        std::memset(ptr, POISON_BYTE, size);
        active_allocs.push_back({ptr, size, align});
      } else {
        action = 100;
      }
    }

    if (action >= 60 && !active_allocs.empty()) {
      AllocRecord record = active_allocs.back();

      if (!verify_mem(record.ptr, record.size, POISON_BYTE)) {
        std::cerr << "Stack corruption detected at iteration " << i << std::endl;
        std::abort();
      }

      alloc.free_raw(record.ptr);
      active_allocs.pop_back();
    }
  }
}

template <class Allocator>
void stress_test_arena(std::size_t heap_size, std::size_t iters) {
  Allocator alloc(heap_size);

  std::mt19937 rng(7331);
  std::uniform_int_distribution<std::size_t> size_dist(1, 4096);
  std::vector<std::size_t> aligns = {8, 16, 32, 64};
  std::uniform_int_distribution<int> align_dist(0, aligns.size() - 1);

  std::vector<AllocRecord> active_allocs;
  constexpr std::uint8_t POISON_BYTE = 0xDD;

  std::size_t total_allocs = 0;

  while (total_allocs < iters) {
    std::size_t size = size_dist(rng);
    std::size_t align = aligns[align_dist(rng)];

    void* ptr = alloc.alloc_raw(size, align);

    if (ptr) {
      assert((reinterpret_cast<std::uintptr_t>(ptr) % align) == 0);
      std::memset(ptr, POISON_BYTE, size);
      active_allocs.push_back({ptr, size, align});
      total_allocs++;
    } else {
      for (const auto& record : active_allocs)
        if (!verify_mem(record.ptr, record.size, POISON_BYTE)) {
          std::cerr << "Arena corruption detected before clear." << std::endl;
          std::abort();
        }

      alloc.clear();
      active_allocs.clear();
    }
  }
}

int main() {
  constexpr std::size_t MB = 1024 * 1024;
  constexpr std::size_t ITERS = 1000000;

  stress_test_stack<oo_alloc::StackAllocator>(8 * MB, ITERS);
  stress_test_arena<oo_alloc::ArenaAllocator>(8 * MB, ITERS);
}
