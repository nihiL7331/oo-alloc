#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <bit>
#include <cstdint>

namespace oo_alloc {

class BuddyAllocator: public IAllocator {
private:
  struct AllocHeader {
    std::uint8_t data; // MSB - is_free, rest - order

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
  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override;
  bool  init(std::size_t size) override;
  void clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_total_size; }

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

    std::size_t offset = reinterpret_cast<std::uint8_t *>(block) - 
                          static_cast<std::uint8_t *>(m_start_ptr);
    offset ^= (MIN_BLOCK_SIZE << order);

    std::uint8_t* buddy_ptr = static_cast<std::uint8_t *>(m_start_ptr) + offset;
    return reinterpret_cast<FreeBlock *>(buddy_ptr);
  }

  void split_block(std::uint8_t order) noexcept;
};

}
