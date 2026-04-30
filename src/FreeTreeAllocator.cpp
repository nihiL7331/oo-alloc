#include "oo_alloc/FreeTreeAllocator.hpp"
#include "oo_alloc/utils.hpp"
#include "internal/RBTree.hpp"
#include <cassert>
#include <new>

namespace oo_alloc {

FreeTreeAllocator::~FreeTreeAllocator() {
  if (m_start_ptr != nullptr) {
    utils::os_free(m_start_ptr, m_total_size);
    m_start_ptr = nullptr;
  }
}

FreeTreeAllocator::AllocHeader* FreeTreeAllocator::coalesce(AllocHeader* header) {
  (void)header;
  assert(false && "TODO");
}

void* FreeTreeAllocator::alloc(std::size_t size, std::size_t align) {
  (void)size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

void FreeTreeAllocator::free(void* ptr) {
  (void)ptr;
  assert(false && "TODO");
}

bool FreeTreeAllocator::init(std::size_t size) {
  // this allocator needs additional data
  // for holding the red-black tree structure.
  // this data will be allocated on the beginning
  // of the initialized memory page.
  constexpr std::size_t min_size = sizeof(internal::RBTree)
    + sizeof(AllocHeader) + sizeof(internal::RBTree::Node)
    + sizeof(AllocFooter);
  // too small to allocate anything
  if (size < min_size) 
    return false;

  m_total_size = utils::align_up(size, utils::page_size());
  m_start_ptr = utils::os_alloc(m_total_size);
  if (m_start_ptr == nullptr)
    return false;

  m_free_tree = new (m_start_ptr) internal::RBTree();

  clear();

  return true;
}

void FreeTreeAllocator::clear() {
  assert(false && "TODO");
}

void* FreeTreeAllocator::realloc(void* ptr, std::size_t old_size, 
              std::size_t new_size, std::size_t align) {
  (void)ptr; (void)old_size; (void)new_size; (void)align;
  assert(false && "TODO");
  return nullptr;
}

}
