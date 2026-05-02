#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace oo_alloc {

class BuddyAllocator;

class SlabAllocator: public IAllocator {
private:
  struct SlabHeader {
    SlabHeader* prev;
    SlabHeader* next;

    void* free_list_head;
    std::uint16_t used;
    std::uint16_t capacity;
  };
  struct CacheManager {
    std::size_t object_size;

    SlabHeader* empty_slabs;
    SlabHeader* partial_slabs;
    SlabHeader* full_slabs;

    void init(std::size_t size) {
      object_size = size;
      partial_slabs = nullptr;
      empty_slabs = nullptr;
      full_slabs = nullptr;
    }
  };

  static constexpr std::uint8_t NUM_CACHES = 9;
  static constexpr std::uint8_t MIN_CACHE_ORDER = 3;

  std::size_t m_total_size;
  std::size_t m_page_size;
  BuddyAllocator* m_base_allocator;
  std::array<CacheManager, NUM_CACHES> m_caches;

public:
  SlabAllocator();
  ~SlabAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override; 
  bool  init(std::size_t size) override;
  void  clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return m_total_size; }

private: // helpers
  inline std::uint8_t size_to_cache_idx(std::size_t size) const noexcept {
    if (size <= (1 << MIN_CACHE_ORDER))
      return 0;

    std::size_t round_size = std::bit_ceil(size);
    std::uint8_t order = static_cast<std::uint8_t>(std::countr_zero(round_size));

    return order - MIN_CACHE_ORDER;
  }

  SlabHeader* init_slab(std::uint8_t cache_idx) noexcept;

};

}
