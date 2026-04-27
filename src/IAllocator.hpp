#pragma once
#include <cstddef>
#include <cstdint>

namespace oo_alloc {

class IAllocator {
public:
  virtual ~IAllocator() = default;

  virtual void* alloc(std::size_t size, std::uint8_t align) = 0;
  virtual void free(void* ptr) = 0;
  virtual bool init(std::size_t size) = 0;
  virtual void clear() = 0;
};

}
