#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>
#include <cstdint>

namespace oo_alloc {

class IAllocator {
public:
  IAllocator() = default;
  virtual ~IAllocator() = default;

  IAllocator(const IAllocator&) = delete;
  IAllocator& operator=(const IAllocator&) = delete;
  IAllocator(IAllocator&&) = delete;
  IAllocator& operator=(IAllocator&&) = delete;

  virtual void* alloc_raw(std::size_t size, std::size_t align) = 0;
  virtual void free_raw(void* ptr) = 0;
  virtual void clear() = 0;
  virtual std::size_t capacity() const = 0;
  virtual bool owns(void* ptr) const = 0;

  template<typename T, typename... Args>
  requires (!std::is_array_v<T>)
  T* make(Args&&... args) {
    void* raw_mem = this->alloc_raw(sizeof(T), alignof(T));
    if (raw_mem == nullptr)
      return nullptr;

    try {
      return new (raw_mem) T(std::forward<Args>(args)...);
    } catch (...) {
      this->free_raw(raw_mem);
      throw;
    }
  }

  template<typename T>
  requires std::is_unbounded_array_v<T>
  std::remove_extent_t<T>* make(std::size_t count) {
    using ElemType = std::remove_extent_t<T>;

    if (count == 0 || count > SIZE_MAX / sizeof(ElemType))
      return nullptr;

    void* raw_mem = this->alloc_raw(count * sizeof(ElemType), alignof(ElemType));
    if (raw_mem == nullptr)
      return nullptr;

    ElemType* elem_ptr = static_cast<ElemType *>(raw_mem);
    std::size_t constructed = 0;
    try {
      for (; constructed < count; ++constructed)
        new (&elem_ptr[constructed]) ElemType();
    } catch (...) {
      while (constructed > 0)
        elem_ptr[--constructed].~ElemType();
      this->free_raw(raw_mem);
      throw;
    }

    return elem_ptr;
  }

  template<typename T>
  void destroy(T* ptr) {
    if (ptr == nullptr)
      return;

    ptr->~T();
    this->free_raw(ptr);
  }

  template<typename T>
  void destroy(T* ptr, std::size_t count) {
    if (ptr == nullptr)
      return;

    for (std::size_t i = count; i > 0; --i)
      ptr[i - 1].~T();

    this->free_raw(ptr);
  }
};

}
