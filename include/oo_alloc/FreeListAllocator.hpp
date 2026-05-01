#include "oo_alloc/IAllocator.hpp"
#include <cstdint>

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

private: // helpers
  void* shrink_in_place(void* ptr, AllocHeader* header, std::size_t old_size, std::size_t align_new_size);
  void* expand_right(void* ptr, AllocHeader* header, std::size_t old_size, 
                     std::size_t align_new_size, FreeBlock* right_block, FreeBlock* right_prev);
  void* expand_left(void* ptr, AllocHeader* header, std::size_t old_size,
                    std::size_t align_new_size, FreeBlock* left_block, FreeBlock* left_prev);
  void* expand_both(void* ptr, AllocHeader* header, std::size_t old_size, std::size_t align_new_size, 
                    FreeBlock* left_block, FreeBlock* left_prev, FreeBlock* right_block);

  inline AllocHeader* get_header(void* data_ptr) const noexcept {
    return reinterpret_cast<AllocHeader *>(
      static_cast<std::uint8_t *>(data_ptr) - sizeof(AllocHeader)
    );
  }
  inline std::uint8_t* get_physical_end(void* data_ptr, std::size_t size) const noexcept {
    return static_cast<std::uint8_t *>(data_ptr) + size;
  }
  inline bool aligned(void* ptr, std::size_t align) const noexcept {
    return reinterpret_cast<std::uintptr_t>(ptr) % align == 0;
  }

  inline bool touches_left(FreeBlock* left_block, AllocHeader* target_header) const noexcept {
    if (left_block == nullptr)
      return false;

    std::uint8_t* left_end = reinterpret_cast<std::uint8_t *>(left_block) + left_block->size;
    return left_end == reinterpret_cast<std::uint8_t *>(target_header);
  }

  inline bool touches_right(void* data_ptr, std::size_t old_size, FreeBlock* right_block) const noexcept {
    if (right_block == nullptr)
      return false;

    std::uint8_t* data_end = get_physical_end(data_ptr, old_size);
    return data_end == reinterpret_cast<std::uint8_t *>(right_block);
  }
};

}
