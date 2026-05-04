#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/SlabAllocator.hpp"
#include <benchmark/benchmark.h>
#include <vector>

using namespace oo_alloc;
constexpr std::size_t MB = 1024 * 1024;

template <class Allocator>
static void bm_frag_search(benchmark::State& state) {
  Allocator alloc(512 * MB);
  const size_t N = state.range(0);

  std::vector<void *> to_keep;
  std::vector<void *> to_free;

  for (size_t i = 0; i < N; ++i) {
    to_keep.push_back(alloc.alloc_raw(16, 8));
    to_free.push_back(alloc.alloc_raw(16, 8));
  }

  for (void* p : to_free)
    alloc.free_raw(p);

  for (auto _ : state) {
    void* ptr = alloc.alloc_raw(64, 8);
    benchmark::DoNotOptimize(ptr);
    alloc.free_raw(ptr);
  }

  for (void* p : to_keep)
    alloc.free_raw(p);

  state.SetComplexityN(N);
}

static void bm_frag_search_slab(benchmark::State& state) {
  BuddyAllocator buddy(512 * MB);
  SlabAllocator slab(&buddy);
  
  const size_t N = state.range(0);

  std::vector<void *> to_keep;
  std::vector<void *> to_free;

  for (size_t i = 0; i < N; ++i) {
    to_keep.push_back(slab.alloc_raw(16, 8));
    to_free.push_back(slab.alloc_raw(16, 8));
  }

  for (void* p : to_free)
    slab.free_raw(p);

  for (auto _ : state) {
    void* ptr = slab.alloc_raw(64, 8);
    benchmark::DoNotOptimize(ptr);
    slab.free_raw(ptr);
  }

  for (void* p : to_keep)
    slab.free_raw(p);

  state.SetComplexityN(N);
}

// measures the raw overhead of pointer arithmetic in bump allocators
template <class Allocator>
static void bm_seq_bump(benchmark::State& state) {
    Allocator alloc(256 * MB);

    for (auto _ : state) {
      void* ptr = alloc.alloc_raw(64, 8);
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
    Allocator alloc(64 * MB);

    for (auto _ : state) {
      void* ptr = alloc.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptr);
      alloc.free_raw(ptr);
    }
}

// pool allocator gets its own signature since it requires block size constructor
static void bm_recycle_pool(benchmark::State& state) {
    PoolAllocator alloc(64, 64, 1024);

    for (auto _ : state) {
      void* ptr = alloc.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptr);
      alloc.free_raw(ptr);
    }
}

static void bm_recycle_slab(benchmark::State& state) {
  BuddyAllocator buddy(64 * MB);
  SlabAllocator slab(&buddy);

  for (auto _ : state) { 
    void* ptr = slab.alloc_raw(64, 8);
    benchmark::DoNotOptimize(ptr);
    slab.free_raw(ptr);
  }
}

BENCHMARK(bm_frag_search<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oN);
BENCHMARK(bm_frag_search<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oLogN);
BENCHMARK(bm_frag_search<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
BENCHMARK(bm_frag_search_slab)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);

BENCHMARK(bm_seq_bump<ArenaAllocator>);
BENCHMARK(bm_seq_bump<StackAllocator>);

BENCHMARK(bm_recycle_pool);
BENCHMARK(bm_recycle_dynamic<FreeListAllocator>);
BENCHMARK(bm_recycle_dynamic<FreeTreeAllocator>);
BENCHMARK(bm_recycle_dynamic<BuddyAllocator>);
BENCHMARK(bm_recycle_slab);

BENCHMARK_MAIN();
