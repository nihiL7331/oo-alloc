#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/FreeTreeAllocator.hpp"
#include <benchmark/benchmark.h>

using namespace oo_alloc;
constexpr std::size_t MB = 1024 * 1024;

template <class Allocator>
static void bm_frag_search(benchmark::State& state) {
  Allocator alloc;
  alloc.init(512 * MB);
  const size_t N = state.range(0);

  std::vector<void *> to_keep;
  std::vector<void *> to_free;

  for (size_t i = 0; i < N; ++i) {
    to_keep.push_back(alloc.alloc(16, 8));
    to_free.push_back(alloc.alloc(16, 8));
  }

  for (void* p : to_free)
    alloc.free(p);

  for (auto _ : state) {
    void* ptr = alloc.alloc(64, 8);
    benchmark::DoNotOptimize(ptr);
    alloc.free(ptr);
  }

  for (void* p : to_keep)
    alloc.free(p);

  state.SetComplexityN(N);
}

// measures the raw overhead of pointer arithmetic in bump allocators
template <class Allocator>
static void bm_seq_bump(benchmark::State& state) {
    Allocator alloc;
    alloc.init(256 * MB);

    for (auto _ : state) {
      void* ptr = alloc.alloc(64, 8);
      benchmark::DoNotOptimize(ptr);

      if (!ptr) {
        state.PauseTiming();
        alloc.clear();
        state.ResumeTiming();
      }
    }
}

// measures how fast can allocate and free the exact same size
template <class Allocator>
static void bm_recycle_dynamic(benchmark::State& state) {
    Allocator alloc;
    alloc.init(64 * MB);

    for (auto _ : state) {
      void* ptr = alloc.alloc(64, 8);
      benchmark::DoNotOptimize(ptr);
      alloc.free(ptr);
    }
}

// pool allocator gets its own signature since it requires block size constructor
static void bm_recycle_pool(benchmark::State& state) {
    PoolAllocator alloc(64, 64);
    alloc.init(64 * MB); 

    for (auto _ : state) {
      void* ptr = alloc.alloc(64, 8);
      benchmark::DoNotOptimize(ptr);
      alloc.free(ptr);
    }
}

BENCHMARK(bm_frag_search<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oN);
BENCHMARK(bm_frag_search<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oLogN);

BENCHMARK(bm_seq_bump<ArenaAllocator>);
BENCHMARK(bm_seq_bump<StackAllocator>);

BENCHMARK(bm_recycle_pool);
BENCHMARK(bm_recycle_dynamic<FreeListAllocator>);
BENCHMARK(bm_recycle_dynamic<FreeTreeAllocator>);

BENCHMARK_MAIN();
