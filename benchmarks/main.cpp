#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/SegregatedAllocator.hpp"
#include "oo_alloc/TrackingAllocator.hpp"
#include <benchmark/benchmark.h>
#include <cstdlib>
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

static void bm_seq_bump_tracking(benchmark::State& state) {
  ArenaAllocator base(256 * MB);
  TrackingAllocator tracking(base);

  for (auto _ : state) {
    void* ptr = tracking.alloc_raw(64, 8);
    benchmark::DoNotOptimize(ptr);

    if (!ptr) {
      state.PauseTiming();
      tracking.clear();
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

static void bm_recycle_malloc(benchmark::State& state) {
  for (auto _ : state) {
    void* ptr = std::malloc(64);
    benchmark::DoNotOptimize(ptr);
    std::free(ptr);
  }
}

template <class Allocator>
static void bm_variable_sizes(benchmark::State& state) {
  Allocator alloc(128 * MB);

  std::vector<std::size_t> rand_sizes;
  rand_sizes.reserve(1024);
  for (int i = 0; i < 1024; ++i)
    rand_sizes.push_back(16 + (std::rand() % (512 - 16 + 1)));

  std::size_t idx = 0;
  for (auto _ : state) {
    std::size_t size = rand_sizes[++idx % 1024];
    void* ptr = alloc.alloc_raw(size, 8);
    benchmark::DoNotOptimize(ptr);
    alloc.free_raw(ptr);
  }
}

static void bm_variable_sizes_malloc(benchmark::State& state) {
  std::vector<std::size_t> rand_sizes;
  rand_sizes.reserve(1024);
  for (int i = 0; i < 1024; ++i)
    rand_sizes.push_back(16 + (std::rand() % (512 - 16 + 1)));

  std::size_t idx = 0;
  for (auto _ : state) {
    std::size_t size = rand_sizes[++idx % 1024];
    void* ptr = std::malloc(size);
    benchmark::DoNotOptimize(ptr);
    std::free(ptr);
  }
}

template <class Allocator>
static void bm_random_churn(benchmark::State& state) {
  Allocator alloc(512 * MB);
  
  std::vector<void*> active_ptrs;
  active_ptrs.reserve(1024);
  for(int i = 0; i < 1024; ++i)
    active_ptrs.push_back(alloc.alloc_raw(64, 8));

  std::vector<int> random_indices;
  random_indices.reserve(16384);
  for(int i = 0; i < 16384; ++i)
    random_indices.push_back(std::rand() % 1024);

  std::size_t idx = 0;
  for (auto _ : state) {
    int target_idx = random_indices[++idx % 16384];
    alloc.free_raw(active_ptrs[target_idx]);
    
    void* new_ptr = alloc.alloc_raw(64, 8);
    benchmark::DoNotOptimize(new_ptr);
    active_ptrs[target_idx] = new_ptr;
  }

  for(void* ptr : active_ptrs)
    if (ptr) 
      alloc.free_raw(ptr);
}

static void bm_random_churn_malloc(benchmark::State& state) {
  std::vector<void*> active_ptrs;
  active_ptrs.reserve(1024);
  for(int i = 0; i < 1024; ++i)
    active_ptrs.push_back(std::malloc(64));

  std::vector<int> random_indices;
  random_indices.reserve(16384);
  for(int i = 0; i < 16384; ++i)
    random_indices.push_back(std::rand() % 1024);

  std::size_t idx = 0;
  for (auto _ : state) {
    int target_idx = random_indices[++idx % 16384];
    std::free(active_ptrs[target_idx]);
    
    void* new_ptr = std::malloc(64);
    benchmark::DoNotOptimize(new_ptr);
    active_ptrs[target_idx] = new_ptr;
  }

  for(void* ptr : active_ptrs)
    if (ptr) 
      std::free(ptr);
}

BENCHMARK(bm_frag_search<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oN);
BENCHMARK(bm_frag_search<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oLogN);
BENCHMARK(bm_frag_search<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
BENCHMARK(bm_frag_search<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
BENCHMARK(bm_frag_search_slab)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);

BENCHMARK(bm_seq_bump<ArenaAllocator>);
BENCHMARK(bm_seq_bump<StackAllocator>);
BENCHMARK(bm_seq_bump_tracking);

BENCHMARK(bm_recycle_pool);
BENCHMARK(bm_recycle_slab);
BENCHMARK(bm_recycle_dynamic<BuddyAllocator>);
BENCHMARK(bm_recycle_dynamic<SegregatedAllocator>);
BENCHMARK(bm_recycle_dynamic<FreeTreeAllocator>);
BENCHMARK(bm_recycle_dynamic<FreeListAllocator>);
BENCHMARK(bm_recycle_malloc);

BENCHMARK(bm_variable_sizes<BuddyAllocator>);
BENCHMARK(bm_variable_sizes<SegregatedAllocator>);
BENCHMARK(bm_variable_sizes<FreeTreeAllocator>);
BENCHMARK(bm_variable_sizes<FreeListAllocator>);
BENCHMARK(bm_variable_sizes_malloc);

BENCHMARK(bm_random_churn<BuddyAllocator>);
BENCHMARK(bm_random_churn<SegregatedAllocator>);
BENCHMARK(bm_random_churn<FreeTreeAllocator>);
BENCHMARK(bm_random_churn<FreeListAllocator>);
BENCHMARK(bm_random_churn_malloc);

BENCHMARK_MAIN();
