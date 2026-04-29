#include "oo_alloc/IAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/FreeListAllocator.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>

class Stopwatch {
private:
  std::string m_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

public:
  Stopwatch(const std::string& name) : m_name(name) {
    m_start = std::chrono::high_resolution_clock::now();
  }

  ~Stopwatch() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();

    std::cout << m_name << " took: " << dur << "us\n";
  }
};

// wrapper for built-in malloc for comparison
class MallocAllocator : public oo_alloc::IAllocator {
public:
  ~MallocAllocator() override = default;

  void* alloc(std::size_t size, std::size_t _) override { return std::malloc(size); }
  void  free(void* ptr) override { std::free(ptr); }
  bool  init(std::size_t _) override { return true; }
  void clear() override {}
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override { return std::realloc(ptr, old_size); };
  std::size_t capacity() const override { return 0; }
};

void run_test(oo_alloc::IAllocator& allocator, const std::string& name, int iters, std::size_t alloc_size, std::size_t align, bool is_stack_or_arena = false) {
  // pre-alloc to not get vector overhead in benchmark
  std::vector<void *> ptrs(iters, nullptr);

  {
    Stopwatch timer(name);

    for (int i = 0; i < iters; ++i) {
      ptrs[i] = allocator.alloc(alloc_size, align);
      
      // volatile to discourage any optimizations
      if (ptrs[i] != nullptr) {
          *static_cast<volatile char*>(ptrs[i]) = 'x'; 
      }
    }

    // force fragmentation for freelist and malloc
    if (!is_stack_or_arena) {
        // deterministic seed so that every allocator gets equally shuffled
        std::mt19937 gen(42); 
        std::shuffle(ptrs.begin(), ptrs.end(), gen);
    }

    for (int i = 0; i < iters; ++i) {
      allocator.free(ptrs[i]);
    }
  }
}

#ifndef ITERS
#define ITERS 100000
#endif

void run_warmup(int iters) {
  std::vector<void*> ptrs(iters, nullptr);
  
  for (int i = 0; i < iters; ++i) {
    ptrs[i] = std::malloc(32);
  }
  
  for (int i = iters - 1; i >= 0; --i) {
    std::free(ptrs[i]);
  }
}

int main() {
  run_warmup(ITERS);

  std::size_t large_size = 1024;
  std::uint8_t large_align = 16;
  std::size_t large_total_mem = ITERS * (large_size + 16);

  // std::malloc large
  MallocAllocator large_malloc;
  run_test(large_malloc, "std::malloc (L)", ITERS, large_size, large_align);

  // arena large
  oo_alloc::ArenaAllocator large_arena;
  large_arena.init(large_total_mem);
  run_test(large_arena, "arena (L)", ITERS, large_size, large_align);

  // pool large
  oo_alloc::PoolAllocator large_pool(large_size, large_align);
  large_pool.init(large_total_mem);
  run_test(large_pool, "pool (L)", ITERS, large_size, large_align);

  // stack large
  oo_alloc::StackAllocator large_stack;
  large_stack.init(large_total_mem);
  run_test(large_stack, "stack (L)", ITERS, large_size, large_align);

  // free-list large
  oo_alloc::FreeListAllocator large_free;
  large_free.init(large_total_mem);
  run_test(large_free, "free-list (L)", ITERS, large_size, large_align);

  std::size_t small_size = 32;
  std::uint8_t small_align = 8;
  std::size_t small_total_mem = ITERS * (small_size + 16);

  // std::malloc small
  MallocAllocator small_malloc;
  run_test(small_malloc, "std::malloc (S)", ITERS, small_size, small_align);

  // arena small
  oo_alloc::ArenaAllocator small_arena;
  small_arena.init(small_total_mem);
  run_test(small_arena, "arena (S)", ITERS, small_size, small_align);

  // pool small
  oo_alloc::PoolAllocator small_pool(small_size, small_align);
  small_pool.init(small_total_mem);
  run_test(small_pool, "pool (S)", ITERS, small_size, small_align);

  // stack small
  oo_alloc::StackAllocator small_stack;
  small_stack.init(small_total_mem);
  run_test(small_stack, "stack (S)", ITERS, small_size, small_align);

  // free-list small
  oo_alloc::FreeListAllocator small_free;
  small_free.init(small_total_mem);
  run_test(small_free, "free-list (S)", ITERS, small_size, small_align);
}
