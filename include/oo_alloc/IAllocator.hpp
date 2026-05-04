#pragma once
#include <cstddef>

namespace oo_alloc {

class IAllocator {
public:
  virtual ~IAllocator() = default;

  IAllocator(const IAllocator&) = delete;
  IAllocator& operator=(const IAllocator&) = delete;

  IAllocator(IAllocator&&) noexcept = default;
  IAllocator& operator=(IAllocator&&) noexcept = default;

  virtual void* alloc(std::size_t size, std::size_t align) = 0;
  virtual void free(void* ptr) = 0;
  virtual bool init(std::size_t size) = 0;
  virtual void clear() = 0;
  virtual void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align) = 0;
  virtual std::size_t capacity() const = 0;
};

}
