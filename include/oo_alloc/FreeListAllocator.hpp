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
  explicit FreeListAllocator(std::size_t size);
  ~FreeListAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  void  clear() override;
  std::size_t capacity() const override { return m_total_size; }

private: // helpers
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
};

}
