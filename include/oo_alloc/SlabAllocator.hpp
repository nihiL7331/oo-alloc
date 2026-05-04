#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace oo_alloc {

class SlabAllocator: public IAllocator {
private:
  struct SlabHeader {
    SlabHeader* prev;
    SlabHeader* next;

    void* free_list_head;
    std::uint16_t used;
    std::uint16_t capacity;
    std::uint16_t cache_idx;
    std::uint16_t id;
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
  static constexpr std::uint16_t SLAB_ID = 0x51AB; 

  std::size_t m_page_size;
  IAllocator* m_base_allocator;
  std::array<CacheManager, NUM_CACHES> m_caches;

public:
  explicit SlabAllocator(IAllocator* base_allocator);
  ~SlabAllocator() override;

  void* alloc(std::size_t size, std::size_t align) override;
  void  free(void* ptr) override; 
  void  clear() override;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) override;
  std::size_t capacity() const override { return SIZE_MAX; }

private: // helpers
  inline std::uint8_t size_to_cache_idx(std::size_t size) const noexcept {
    if (size <= (1 << MIN_CACHE_ORDER))
      return 0;

    std::size_t round_size = std::bit_ceil(size);
    std::uint8_t order = static_cast<std::uint8_t>(std::countr_zero(round_size));

    return order - MIN_CACHE_ORDER;
  }

  inline void push_to_list(SlabHeader*& list_head, SlabHeader* slab) noexcept {
    if (slab == nullptr)
      return;

    slab->next = list_head;
    slab->prev = nullptr;

    if (list_head != nullptr)
      list_head->prev = slab;

    list_head = slab;
  }

  inline void remove_from_list(SlabHeader*& list_head, SlabHeader* slab) noexcept {
    if (slab == nullptr)
      return;

    if (slab->prev != nullptr)
      slab->prev->next = slab->next;
    else
      list_head = slab->next;

    if (slab->next != nullptr)
      slab->next->prev = slab->prev;

    slab->prev = nullptr;
    slab->next = nullptr;
  }

  inline void clear_slab_list(SlabHeader*& list_head) noexcept {
    SlabHeader* curr = list_head;
    SlabHeader* next = nullptr;

    while (curr != nullptr) {
      next = curr->next;
      m_base_allocator->free(curr);
      curr = next;
    }

    list_head = nullptr;
  }

  SlabHeader* init_slab(std::uint8_t cache_idx) noexcept;

};

}
