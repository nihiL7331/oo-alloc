#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <bit>
#include <cstdint>

namespace oo_alloc {

class BuddyAllocator: public IAllocator {
private:
  struct AllocHeader {
    std::uint8_t data; // MSB - is_free, rest - order
    std::uint16_t offset;

    bool free() const noexcept { return data & 0x80; }
    std::uint8_t order() const noexcept { return data & 0x7F; }

    void set_free(bool free) noexcept {
      if (free)
        data |= 0x80;
      else
        data &= 0x7F;
    }
    void set_order(std::uint8_t order) noexcept {
      data = (data & 0x80) | (order & 0x7F);
    }
  };
  struct FreeBlock {
    AllocHeader header;
    FreeBlock* prev;
    FreeBlock* next;
  };

  static constexpr std::size_t MIN_BLOCK_SIZE = 32;
  static constexpr std::uint8_t MAX_ORDER = 32;

  void*       m_start_ptr;
  std::size_t m_total_size;
  std::array<FreeBlock*, MAX_ORDER> m_free_lists;

public:
  explicit BuddyAllocator(std::size_t size);
  ~BuddyAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free_raw(void* ptr) override;
  void clear() override;
  std::size_t capacity() const override { return m_total_size; }
  bool owns(void* ptr) const override;

private: // helpers
  inline std::uint8_t size_to_order(std::size_t size) const noexcept {
    size += sizeof(AllocHeader);
    size = std::bit_ceil(size);
    if (size < MIN_BLOCK_SIZE)
      size = MIN_BLOCK_SIZE;

    return static_cast<std::uint8_t>(std::countr_zero(size) - std::countr_zero(MIN_BLOCK_SIZE));
  }

  inline FreeBlock* get_buddy(FreeBlock* block, std::uint8_t order) const noexcept {
    if (block == nullptr)
      return nullptr;

    std::uintptr_t block_addr = reinterpret_cast<std::uintptr_t>(block);
    std::uintptr_t start_addr = reinterpret_cast<std::uintptr_t>(m_start_ptr);

    std::uintptr_t rel_offset = block_addr - start_addr;

    std::size_t block_size = MIN_BLOCK_SIZE << order;
    std::uintptr_t buddy_offset = rel_offset ^ block_size;

    std::uintptr_t buddy_addr = start_addr + buddy_offset;

    return reinterpret_cast<FreeBlock *>(buddy_addr);
  }

  void split_block(std::uint8_t order) noexcept;
};

}
