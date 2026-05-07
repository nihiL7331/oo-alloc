#include "oo_alloc/ArenaAllocator.hpp"
#include "oo_alloc/StackAllocator.hpp"
#include "oo_alloc/PoolAllocator.hpp"
#include "oo_alloc/FreeListAllocator.hpp"
#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/BuddyAllocator.hpp"
#include "oo_alloc/SlabAllocator.hpp"
#include "oo_alloc/SegregatedAllocator.hpp"
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

  for (std::size_t i = 0; i < N; ++i) {
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

static void bm_frag_search_malloc(benchmark::State& state) {
  std::vector<void *> to_keep;
  std::vector<void *> to_free;

  const size_t N = state.range(0);

  for (std::size_t i = 0; i < N; ++i) {
    to_keep.push_back(std::malloc(16));
    to_free.push_back(std::malloc(16));
  }

  for (void* p : to_free)
    std::free(p);

  for (auto _ : state) {
    void* ptr = std::malloc(64);
    benchmark::DoNotOptimize(ptr);
    std::free(ptr);
  }

  for (void* p : to_keep)
    std::free(p);

  state.SetComplexityN(N);
}

// measures the raw overhead of pointer arithmetic in bump allocators
template <class Allocator>
static void bm_seq_bump(benchmark::State& state) {
  Allocator alloc(256 * MB);
  const std::size_t N = state.range(0);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      void* ptr = alloc.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptr);
    }

    alloc.clear();
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

static void bm_seq_bump_malloc(benchmark::State& state) {
  const std::size_t N = state.range(0);
  std::vector<void *> ptrs(N);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      void* ptr = std::malloc(64);
      benchmark::DoNotOptimize(ptr);
    }

    state.PauseTiming();
    for (void* p : ptrs)
      std::free(p);
    state.ResumeTiming();
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

// measures how fast can allocate and free the exact same size
template <class Allocator>
static void bm_recycle_dynamic(benchmark::State& state) {
  Allocator alloc(64 * MB);
  const std::size_t N = state.range(0);
  std::vector<void *> ptrs(N);

  std::vector<int> free_order(N);
  for (std::size_t i = 0; i < N; ++i)
    free_order[i] = i;

  for (std::size_t i = 0; i < N; ++i)
    std::swap(free_order[i], free_order[std::rand() % N]);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      ptrs[i] = alloc.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptrs[i]);
    }

    for (std::size_t i = 0; i < N; ++i)
      alloc.free_raw(ptrs[free_order[i]]);
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

// pool allocator gets its own signature since it requires block size constructor
static void bm_recycle_pool(benchmark::State& state) {
  PoolAllocator alloc(64, 64, MB);
  const std::size_t N = state.range(0);
  std::vector<void *> ptrs(N);

  std::vector<int> free_order(N);
  for (std::size_t i = 0; i < N; ++i)
    free_order[i] = i;

  for (std::size_t i = 0; i < N; ++i)
    std::swap(free_order[i], free_order[std::rand() % N]);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      ptrs[i] = alloc.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptrs[i]);
    }

    for (std::size_t i = 0; i < N; ++i)
      alloc.free_raw(ptrs[free_order[i]]);
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

static void bm_recycle_slab(benchmark::State& state) {
  BuddyAllocator buddy(64 * MB);
  SlabAllocator slab(&buddy);
  const std::size_t N = state.range(0);
  std::vector<void *> ptrs(N);

  std::vector<int> free_order(N);
  for (std::size_t i = 0; i < N; ++i)
    free_order[i] = i;

  for (std::size_t i = 0; i < N; ++i)
    std::swap(free_order[i], free_order[std::rand() % N]);

  for (auto _ : state) { 
    for (std::size_t i = 0; i < N; ++i) {
      ptrs[i] = slab.alloc_raw(64, 8);
      benchmark::DoNotOptimize(ptrs[i]);
    }

    for (std::size_t i = 0; i < N; ++i)
      slab.free_raw(ptrs[free_order[i]]);
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

static void bm_recycle_malloc(benchmark::State& state) {
  const std::size_t N = state.range(0);
  std::vector<void *> ptrs(N);

  std::vector<int> free_order(N);
  for (std::size_t i = 0; i < N; ++i)
    free_order[i] = i;

  for (std::size_t i = 0; i < N; ++i)
    std::swap(free_order[i], free_order[std::rand() % N]);

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      ptrs[i] = std::malloc(64);
      benchmark::DoNotOptimize(ptrs[i]);
    }

    for (std::size_t i = 0; i < N; ++i)
      std::free(ptrs[free_order[i]]);
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

template <class Allocator>
static void bm_variable_sizes(benchmark::State& state) {
  Allocator alloc(128 * MB);
  const std::size_t N = state.range(0);

  std::vector<std::size_t> rand_sizes(N);
  for (int i = 0; i < N; ++i)
    rand_sizes[i] = 16 + (std::rand() % (512 - 16 + 1));

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      void* ptr = alloc.alloc_raw(rand_sizes[i], 8);
      if (ptr == nullptr) {
        state.SkipWithError("Allocation failed! Fragmentation limit reached.");
        break;
      }
      benchmark::DoNotOptimize(ptr);
      alloc.free_raw(ptr);
    }
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

static void bm_variable_sizes_malloc(benchmark::State& state) {
  const std::size_t N = state.range(0);

  std::vector<std::size_t> rand_sizes(N);
  for (int i = 0; i < N; ++i)
    rand_sizes[i] = 16 + (std::rand() % (512 - 16 + 1));

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      void* ptr = std::malloc(rand_sizes[i]);
      benchmark::DoNotOptimize(ptr);
      std::free(ptr);
    }
  }

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

template <class Allocator>
static void bm_random_churn(benchmark::State& state) {
  Allocator alloc(512 * MB);
  const std::size_t N = state.range(0);
  std::vector<void *> slots(N, nullptr);

  for (std::size_t i = 0; i < N/2; ++i)
    slots[i] = alloc.alloc_raw(64, 8);

  for (std::size_t i = 0; i < N; ++i)
    std::swap(slots[i], slots[std::rand() % N]);

  const std::size_t OP_COUNT = 100000;
  std::vector<std::size_t> random_ops(OP_COUNT);
  for (std::size_t i = 0; i < OP_COUNT; ++i)
    random_ops[i] = std::rand() % N;

  std::size_t op_idx = 0;
  
  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      int target_idx = random_ops[++op_idx % OP_COUNT];

      if (slots[target_idx] == nullptr) {
        slots[target_idx] = alloc.alloc_raw(64, 8);
        benchmark::DoNotOptimize(slots[target_idx]);
      } else {
        alloc.free_raw(slots[target_idx]);
        slots[target_idx] = nullptr;
      }
    }
  }

  state.PauseTiming();
  for(void* ptr : slots)
    if (ptr) 
      alloc.free_raw(ptr);
  state.ResumeTiming();

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

static void bm_random_churn_malloc(benchmark::State& state) {
  const std::size_t N = state.range(0);
  std::vector<void *> slots(N, nullptr);

  for (std::size_t i = 0; i < N/2; ++i)
    slots[i] = std::malloc(64);

  for (std::size_t i = 0; i < N; ++i)
    std::swap(slots[i], slots[std::rand() % N]);

  const std::size_t OP_COUNT = 100000;
  std::vector<std::size_t> random_ops(OP_COUNT);
  for (std::size_t i = 0; i < OP_COUNT; ++i)
    random_ops[i] = std::rand() % N;

  std::size_t op_idx = 0;

  for (auto _ : state) {
    for (std::size_t i = 0; i < N; ++i) {
      int target_idx = random_ops[++op_idx % OP_COUNT];

      if (slots[target_idx] == nullptr) {
        slots[target_idx] = std::malloc(64);
        benchmark::DoNotOptimize(slots[target_idx]);
      } else {
        std::free(slots[target_idx]);
        slots[target_idx] = nullptr;
      }
    }
  }

  state.PauseTiming();
  for(void* ptr : slots)
    if (ptr) 
      std::free(ptr);
  state.ResumeTiming();

  state.SetItemsProcessed(state.iterations() * N);
  state.SetComplexityN(N);
}

auto* b_frag_freelist = benchmark::RegisterBenchmark("frag_search/free_list", bm_frag_search<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oN);
auto* b_frag_freetree = benchmark::RegisterBenchmark("frag_search/free_tree", bm_frag_search<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oLogN);
auto* b_frag_seg      = benchmark::RegisterBenchmark("frag_search/segregated", bm_frag_search<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
auto* b_frag_buddy    = benchmark::RegisterBenchmark("frag_search/buddy", bm_frag_search<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
auto* b_frag_slab     = benchmark::RegisterBenchmark("frag_search/slab", bm_frag_search_slab)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);
auto* b_frag_malloc   = benchmark::RegisterBenchmark("frag_search/malloc", bm_frag_search_malloc)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::o1);

auto* b_seq_arena    = benchmark::RegisterBenchmark("seq_bump/arena", bm_seq_bump<ArenaAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_stack    = benchmark::RegisterBenchmark("seq_bump/stack", bm_seq_bump<StackAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_freelist = benchmark::RegisterBenchmark("seq_bump/free_list", bm_seq_bump<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_freetree = benchmark::RegisterBenchmark("seq_bump/free_tree", bm_seq_bump<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_seg      = benchmark::RegisterBenchmark("seq_bump/segregated", bm_seq_bump<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_buddy    = benchmark::RegisterBenchmark("seq_bump/buddy", bm_seq_bump<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_seq_malloc   = benchmark::RegisterBenchmark("seq_bump/malloc", bm_seq_bump_malloc)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);

auto* b_rec_pool     = benchmark::RegisterBenchmark("recycle/pool", bm_recycle_pool)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_seg      = benchmark::RegisterBenchmark("recycle/segregated", bm_recycle_dynamic<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_freetree = benchmark::RegisterBenchmark("recycle/free_tree", bm_recycle_dynamic<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_freelist = benchmark::RegisterBenchmark("recycle/free_list", bm_recycle_dynamic<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_slab     = benchmark::RegisterBenchmark("recycle/slab", bm_recycle_slab)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_buddy    = benchmark::RegisterBenchmark("recycle/buddy", bm_recycle_dynamic<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_rec_malloc   = benchmark::RegisterBenchmark("recycle/malloc", bm_recycle_malloc)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);

auto* b_var_buddy    = benchmark::RegisterBenchmark("var_size/buddy", bm_variable_sizes<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_var_seg      = benchmark::RegisterBenchmark("var_size/segregated", bm_variable_sizes<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_var_freetree = benchmark::RegisterBenchmark("var_size/free_tree", bm_variable_sizes<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_var_freelist = benchmark::RegisterBenchmark("var_size/free_list", bm_variable_sizes<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_var_malloc   = benchmark::RegisterBenchmark("var_size/malloc", bm_variable_sizes_malloc)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);

auto* b_churn_buddy    = benchmark::RegisterBenchmark("churn/buddy", bm_random_churn<BuddyAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_churn_seg      = benchmark::RegisterBenchmark("churn/segregated", bm_random_churn<SegregatedAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_churn_freetree = benchmark::RegisterBenchmark("churn/free_tree", bm_random_churn<FreeTreeAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_churn_freelist = benchmark::RegisterBenchmark("churn/free_list", bm_random_churn<FreeListAllocator>)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);
auto* b_churn_malloc   = benchmark::RegisterBenchmark("churn/malloc", bm_random_churn_malloc)->RangeMultiplier(10)->Range(10, 10000)->Complexity(benchmark::oAuto);

BENCHMARK_MAIN();
