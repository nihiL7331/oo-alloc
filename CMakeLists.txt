cmake_minimum_required(VERSION 3.20)

project(oo-alloc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
  src/ArenaAllocator.cpp
  src/PoolAllocator.cpp
  src/StackAllocator.cpp
  src/TrackingAllocator.cpp
  src/FreeListAllocator.cpp
  src/FreeTreeAllocator.cpp
  src/internal/RBTree.cpp
  src/BuddyAllocator.cpp
  src/SlabAllocator.cpp
  src/SegregatedAllocator.cpp
)

add_library(oo_alloc STATIC ${SOURCE_FILES})

target_include_directories(oo_alloc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_testing()

add_subdirectory(tests)

target_compile_options(oo_alloc PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

include(FetchContent)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.5
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(bench_suite benchmarks/main.cpp)
target_link_libraries(bench_suite PRIVATE oo_alloc benchmark::benchmark)
add_custom_target(run-benchmarks
  COMMAND ./bench_suite
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS bench_suite
  COMMENT "Running benchmarks..."
  USES_TERMINAL
)
