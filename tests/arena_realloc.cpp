#include "oo_alloc/ArenaAllocator.hpp"
#include <cassert>
#include <cstring>

int main() {
  oo_alloc::ArenaAllocator arena;
  bool succ = arena.init(1024);
  assert(succ && "Failed to initialize");

  void* ptr1 = arena.alloc(10, 8);
  void* ptr1_resized = arena.realloc(ptr1, 10, 50, 8);
  assert(ptr1 == ptr1_resized && "Should return the same pointer");

  char* str_ptr = static_cast<char *>(arena.alloc(16, 1));
  std::strcpy(str_ptr, "Hello, World");
  void* block_ptr = arena.alloc(10, 1);
  char* str_ptr_resized = static_cast<char *>(arena.realloc(str_ptr, 16, 64, 1));
  assert(str_ptr != str_ptr_resized && "Should return a new pointer");
  assert(std::strcmp(str_ptr_resized, "Hello, World") == 0 && "memcpy failed to retain data");

  void* overflow_ptr = arena.realloc(str_ptr_resized, 64, 99999, 8);
  assert(overflow_ptr == nullptr && "Should fail to realloc on overflow");
}
