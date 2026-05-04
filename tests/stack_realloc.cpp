#include "oo_alloc/StackAllocator.hpp"
#include <cassert>
#include <cstring>

int main() {
  oo_alloc::StackAllocator stack(1024);

  void *ptr1 = stack.alloc(16, 8);
  void *ptr1_grown = stack.realloc(ptr1, 16, 32, 8);
  assert(ptr1 == ptr1_grown && "Should return the same pointer");

  char *str_ptr = static_cast<char *>(stack.alloc(16, 1));
  std::strcpy(str_ptr, "Hello, World");

  void *block = stack.alloc(16, 8);

  char *str_ptr_grown = static_cast<char *>(stack.realloc(str_ptr, 16, 64, 1));
  assert(str_ptr != str_ptr_grown &&
         "Should return a new pointer");
  assert(std::strcmp(str_ptr_grown, "Hello, World") == 0 &&
         "memcpy failed to retain data");

  void *str_ptr_shrunk = stack.realloc(str_ptr, 16, 8, 1);
  assert(str_ptr == str_ptr_shrunk &&
         "Should return the original pointer");

  void *overflow_ptr = stack.realloc(str_ptr_grown, 64, 99999, 8);
  assert(overflow_ptr == nullptr && "Should fail on overflow realloc");
}
