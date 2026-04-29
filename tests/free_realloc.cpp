#include "oo_alloc/FreeListAllocator.hpp" 
#include <cassert>
#include <cstring>
#include <cstdint>

int main() {
  oo_alloc::FreeListAllocator free;
  bool succ = free.init(1024);
  assert(succ && "Failed to initialize");

  void* ptr1 = free.alloc(200, 8);
  void* ptr1_shrunk = free.realloc(ptr1, 200, 64, 8);
  assert(ptr1 == ptr1_shrunk && "Shrink should happen in-place");
  
  void* ptr_filler = free.alloc(100, 8);
  assert(ptr_filler != nullptr && "Shrink-split failed to reclaim memory");
  free.free(ptr_filler);

  void* ptr2 = free.alloc(64, 8);
  void* ptr3 = free.alloc(64, 8);
  void* ptr4 = free.alloc(64, 8);
  free.free(ptr3);
  void* ptr2_expanded = free.realloc(ptr2, 64, 120, 8);
  assert(ptr2 == ptr2_expanded && "Fast path expand-right failed to stay in-place");

  std::strcpy(static_cast<char*>(ptr2_expanded), "Hello, World");
  void* ptr2_moved = free.realloc(ptr2_expanded, 120, 300, 8);
  assert(ptr2_moved != ptr2_expanded && "Slow path should have moved the alloc");
  assert(std::strcmp(static_cast<char*>(ptr2_moved), "Hello, World") == 0 && "Data corruption during slow path memcpy");

  void* ptr2_aligned = free.realloc(ptr2_moved, 300, 400, 64);
  assert(reinterpret_cast<std::uintptr_t>(ptr2_aligned) % 64 == 0 && "Failed to respect 64-byte alignment");

  void* ptr2_too_big = free.realloc(ptr2_aligned, 400, 5000, 8);
  assert(ptr2_too_big == nullptr && "Should return nullptr on overflow");

  void* left_block = free.alloc(64, 8);
  void* mid_block = free.alloc(64, 8);
  void* right_barrier = free.alloc(64, 8);
  std::strcpy(static_cast<char *>(mid_block), "Hello, World");
  free.free(left_block);
  void* mid_expanded = free.realloc(mid_block, 64, 100, 8);
  assert(mid_expanded != mid_block && "Left expand should change the pointer");
  assert(reinterpret_cast<std::uintptr_t>(mid_expanded) < reinterpret_cast<std::uintptr_t>(mid_block) && "Pointer should move backwards in memory");
  assert(std::strcmp(static_cast<char*>(mid_expanded), "Hello, World") == 0 && "Data corrupted during left expand memmove");

  void* tiny_ptr = free.alloc(64, 8);
  void* tiny_shrunk = free.realloc(tiny_ptr, 64, 60, 8); 
  assert(tiny_ptr == tiny_shrunk && "Tiny shrink should not move the pointer");

  void* blockA = free.alloc(32, 8);
  void* blockB = free.alloc(32, 8);
  void* blockC = free.alloc(32, 8);
  void* blockD = free.alloc(32, 8); 
  std::strcpy(static_cast<char*>(blockB), "Foo, Bar!");
  free.free(blockA);
  free.free(blockC);
  void* b_double_expanded = free.realloc(blockB, 32, 100, 8); 
  assert(b_double_expanded != blockB && "Double expand should change the pointer");
  assert(reinterpret_cast<std::uintptr_t>(b_double_expanded) < reinterpret_cast<std::uintptr_t>(blockB) && "Pointer should shift into the left block");
  assert(std::strcmp(static_cast<char*>(b_double_expanded), "Foo, Bar!") == 0 && "Data corrupted during double expand memmove");
}
