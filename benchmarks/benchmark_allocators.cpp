#include "IAllocator.hpp"
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

