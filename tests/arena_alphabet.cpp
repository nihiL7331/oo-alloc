#include "oo_alloc/ArenaAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::ArenaAllocator arena(1024);

  char* data[26];
  for (int i = 0; i < 26; ++i) {
    data[i] = static_cast<char *>(arena.alloc_raw(sizeof(char), alignof(char)));
    *data[i] = 'A' + i;
  }

  for (int i = 0; i < 26; ++i)
    assert(*data[i] == 'A' + i && "Char data mismatch");

  arena.clear();

  void* start_ptr = arena.alloc_raw(sizeof(char), alignof(char));
  assert(start_ptr != nullptr && "Failed to alloc after clear");
}
