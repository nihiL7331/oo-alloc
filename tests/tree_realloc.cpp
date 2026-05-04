#include "oo_alloc/FreeTreeAllocator.hpp"
#include <cassert>
#include <cstring>

int main() {
  oo_alloc::FreeTreeAllocator allocator(16384);

  void* p1 = allocator.realloc(nullptr, 0, 128, 8);
  assert(p1 != nullptr && "realloc(nullptr) failed to allocate!");

  std::memset(p1, 0xAA, 128);

  void* p1_shrunk = allocator.realloc(p1, 128, 64, 8);
  assert(p1_shrunk == p1 && "Shrinking memory moved the pointer unnecessarily");
  
  std::uint8_t* bytes = static_cast<std::uint8_t *>(p1_shrunk);
  assert(bytes[0] == 0xAA && bytes[63] == 0xAA && "Data corrupted during shrink");

  void* p1_expanded = allocator.realloc(p1_shrunk, 64, 256, 8);
  assert(p1_expanded == p1 && "Failed to expand in place");
  assert(bytes[0] == 0xAA && bytes[63] == 0xAA && "Data corrupted during expand in place");

  void* blocker = allocator.alloc(128, 8);
  assert(blocker != nullptr);

  void* p1_moved = allocator.realloc(p1_expanded, 256, 512, 8);
  assert(p1_moved != nullptr && p1_moved != p1 && "realloc failed to move block when neighbor was allocated");

  std::uint8_t* moved_bytes = static_cast<std::uint8_t *>(p1_moved);
  assert(moved_bytes[0] == 0xAA && moved_bytes[63] == 0xAA && "Data not copied during realloc move");

  void* p_freed = allocator.realloc(p1_moved, 512, 0, 8);
  assert(p_freed == nullptr && "realloc with size 0 did not return nullptr");
}
