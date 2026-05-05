#include "oo_alloc/IAllocator.hpp"
#include <array>
#include <cstddef>

namespace oo_alloc {

class SegregatedAllocator: public IAllocator {
private:
  struct AllocHeader {
    std::size_t size;
    std::size_t pad;
  };
  struct FreeBlock {
    AllocHeader header;
    FreeBlock* next;
  };

  static constexpr std::uint8_t NUM_BUCKETS = 9;
  static constexpr std::uint8_t MIN_BUCKET_ORDER = 3;

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
};

}
