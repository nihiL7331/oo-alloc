#include "IAllocator.hpp"
#include "PoolAllocator.hpp"
#include "StackAllocator.hpp"
#include "ArenaAllocator.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

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

  void* alloc(std::size_t size, std::uint8_t _) override { return std::malloc(size); }
  void  free(void* ptr) override { std::free(ptr); }
  bool  init(std::size_t _) override { return true; }
  void clear() override {}
};

void run_test(oo_alloc::IAllocator& allocator, const std::string& name, int iters, std::size_t alloc_size, std::uint8_t align) {
  // pre-alloc to not get vector overhead in benchmark
  std::vector<void *> ptrs(iters, nullptr);

  {
    Stopwatch timer(name);

    for (int i = 0; i < iters; ++i)
      ptrs[i] = allocator.alloc(alloc_size, align);

    for (int i = iters - 1; i >= 0; --i)
      allocator.free(ptrs[i]);
  }
}

#ifndef ITERS
#define ITERS 100000
#endif

int main() {
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
}
