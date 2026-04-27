add_executable(benchmark_allocators benchmark_allocators.cpp)
target_link_libraries(benchmark_allocators PRIVATE oo_alloc)

set(ITERS "100000" CACHE STRING "Iter count for benchmark loop")
target_compile_definitions(benchmark_allocators PRIVATE ITERS=${ITERS})

add_custom_target(run-benchmarks
  COMMAND benchmark_allocators
  DEPENDS benchmark_allocators
  COMMENT "Running performance benchmarks..."
)
