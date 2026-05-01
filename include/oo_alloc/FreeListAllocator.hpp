#include "oo_alloc/IAllocator.hpp"

namespace oo_alloc {

class FreeListAllocator: public IAllocator {
private:
  struct FreeBlock {
    std::size_t size;
    FreeBlock *next;
  };
  struct AllocHeader {
    std::size_t size;
    std::size_t pad;
  };
  void*       m_start_ptr;
  std::size_t m_total_size;
  FreeBlock* m_free_list_head;

  void coalesce(FreeBlock* prev_block, FreeBlock* free_block);

public:
  FreeListAllocator()
    : m_start_ptr(nullptr), m_total_size(0), m_free_list_head(nullptr) {}

  ~FreeListAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void  clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_total_size; }
};

}
