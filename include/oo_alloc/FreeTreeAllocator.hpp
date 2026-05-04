#pragma once
#include "oo_alloc/IAllocator.hpp"
#include <cstdint>

namespace oo_alloc {

namespace internal {

class RBTree;

}

class FreeTreeAllocator: public IAllocator {
private:
  void*             m_start_ptr;
  std::size_t       m_total_size;
  internal::RBTree* m_free_tree;

public:
  explicit FreeTreeAllocator(std::size_t size);
  ~FreeTreeAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  void  clear() override;

  std::size_t capacity() const noexcept override { return m_total_size; }

private: // metadata management
  struct BlockMetadata {
    std::size_t size_and_flags;

    std::size_t size() const noexcept { return size_and_flags & ~7ULL; }
    bool allocated() const noexcept { return size_and_flags & 1; }
    void state(std::size_t new_size, bool allocated) { size_and_flags = new_size | static_cast<std::size_t>(allocated); }
  };

  using AllocHeader = BlockMetadata;
  using AllocFooter = BlockMetadata;

  inline void update_block(AllocHeader* header, std::size_t new_size, bool allocated) noexcept {
    header->state(new_size, allocated);

    AllocFooter* footer = reinterpret_cast<AllocFooter *>(
      reinterpret_cast<std::uint8_t *>(header) + sizeof(AllocHeader) + new_size
    );

    footer->state(new_size, allocated);
  }

  AllocHeader* coalesce(AllocHeader* header);
};
}
