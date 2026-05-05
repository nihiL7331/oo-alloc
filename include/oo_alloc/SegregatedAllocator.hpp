#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace oo_alloc {

class SegregatedAllocator: public IAllocator {
private:
  struct AllocHeader {
    std::uint8_t order : 7;
    std::uint8_t is_prev_free : 1;
  };
  struct AllocFooter {
    std::uint8_t order;
  };
  struct FreeBlock {
    FreeBlock* prev;
    FreeBlock* next;
  };

  static constexpr std::uint8_t NUM_BUCKETS = 9;
  static constexpr std::uint8_t MIN_BUCKET_ORDER = 5;

  void*       m_start_ptr;
  std::size_t m_total_size;

  std::array<FreeBlock*, NUM_BUCKETS> m_buckets;

public:
  explicit SegregatedAllocator(std::size_t size);
  ~SegregatedAllocator() override;

  void* alloc_raw(std::size_t size, std::size_t align) override;
  void  free_raw(void* ptr) override;
  void clear() override;
  std::size_t capacity() const override { return m_total_size; }
  bool owns(void* ptr) const override;

private: //helpers
  inline std::uint8_t size_to_bucket(std::size_t size) const noexcept {
    if (size <= (1 << MIN_BUCKET_ORDER))
      return 0;

    std::size_t round_size = std::bit_ceil(size);
    std::uint8_t order = static_cast<std::uint8_t>(std::countr_zero(round_size));

    return order - MIN_BUCKET_ORDER;
  }
  inline std::size_t bucket_to_size(std::uint8_t bucket) const noexcept {
    return 1ULL << (MIN_BUCKET_ORDER + bucket);
  }
};

}
