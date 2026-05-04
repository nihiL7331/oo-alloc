#include "oo_alloc/ArenaAllocator.hpp"
#include <cassert>

struct Entity {
  static int count;
  int id;
  Entity() : id(count) { count++; }
  Entity(int id) : id(id) { count++; }
  ~Entity() { count--; }
};
int Entity::count = 0;

int main() {
  oo_alloc::ArenaAllocator arena(1024);

  Entity* entity1 = arena.make<Entity>(42);
  assert(entity1 != nullptr && "Failed to allocate entity1");
  assert(entity1->id == 42 && "Data corruption within entity1");
  assert(Entity::count == 1 && "Failed to call constructor on Entity");

  constexpr std::size_t entities_count = 5;
  Entity* entities = arena.make<Entity[]>(entities_count);
  assert(entities != nullptr);
  assert(Entity::count == 1 + entities_count && "Failed to call constructor on Entity[]");

  arena.destroy(entity1);
  assert(Entity::count == entities_count && "Failed to call destructor on Entity");

  arena.destroy(entity1, entities_count);
  assert(Entity::count == 0 && "Failed to call destructor on Entity[]");

  arena.clear();
  void* first_after_clear = arena.alloc_raw(8, 8);
  assert(first_after_clear == entity1 && "Unexpected memory address after clear");
}
