#include "oo_alloc/ArenaAllocator.hpp"
#include <cassert>
#include <cstdint>

int main() {
  oo_alloc::ArenaAllocator arena;
  bool succ = arena.init(1024);
  assert(succ && "Failed to initialize");
  
  void* ptr1 = arena.alloc(sizeof(char), alignof(char));
  assert(ptr1 != nullptr && "Failed to allocate char");

  void* ptr2 = arena.alloc(sizeof(int), alignof(int));
  assert(ptr2 != nullptr && "Failed to allocate int");

  uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
  assert(addr2 % alignof(int) == 0 && "Failed to align int ptr");

  uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
  assert(addr2 == addr1 + 4 && "Failed to set padding char/int");

  arena.alloc(sizeof(char), alignof(char));

  void* ptr3 = arena.alloc(sizeof(long), alignof(long));
  uintptr_t addr3 = reinterpret_cast<uintptr_t>(ptr3);
  assert(addr3 % alignof(long) == 0 && "Failed to align long ptr");
  assert(addr3 == addr2 + 12 && "Failed to set padding int/long");
}
