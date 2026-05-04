#include "oo_alloc/BuddyAllocator.hpp"
#include <cassert>
#include <cstring>

int main() {
  oo_alloc::BuddyAllocator buddy(1024 * 1024);

  void* ptr1 = buddy.alloc(40, 8);
  assert(ptr1 != nullptr && "Initial alloc failed");
  
  std::memset(ptr1, 0xAA, 40);

  void* ptr2 = buddy.realloc(ptr1, 40, 50, 8);
  assert(ptr1 == ptr2 && "Realloc moved memory when block was large enough");
  
  std::uint8_t* check_ptr = static_cast<std::uint8_t *>(ptr2);
  assert(check_ptr[0] == 0xAA && check_ptr[39] == 0xAA && "Data corrupted during in-place realloc");

  void* ptr3 = buddy.realloc(ptr2, 50, 100, 8);
  assert(ptr3 != ptr2 && "Realloc failed to allocate a new block for expansion");
  
  check_ptr = static_cast<std::uint8_t *>(ptr3);
  assert(check_ptr[0] == 0xAA && check_ptr[39] == 0xAA && "Data lost during allocate-copy-free cycle");
}
