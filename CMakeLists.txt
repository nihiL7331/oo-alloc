cmake_minimum_required(VERSION 3.20)

project(oo-alloc VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
  src/ArenaAllocator.cpp
  src/PoolAllocator.cpp
  src/StackAllocator.cpp
  src/TrackingAllocator.cpp
)

add_library(oo_alloc STATIC ${SOURCE_FILES})

target_include_directories(oo_alloc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

enable_testing()

add_subdirectory(tests)
add_subdirectory(benchmarks)

target_compile_options(oo_alloc PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
