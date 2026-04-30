#pragma once
#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

namespace internal {

class RBTree;

}

class FreeTreeAllocator: public IAllocator {
private:
  struct AllocHeader {
    std::size_t size;
    std::size_t pad;
  };
  struct AllocFooter {
    std::size_t size;
    // is_free = LSB(size)
  };
  void*             m_start_ptr;
  std::size_t       m_total_size;
  internal::RBTree* m_free_tree;

public:
  FreeTreeAllocator() 
    : m_start_ptr(nullptr), m_total_size(0), m_free_tree(nullptr) {}
  ~FreeTreeAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void  clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_total_size; }
};

}
