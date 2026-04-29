#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
  #include <memoryapi.h>
#else
  #include <sys/mman.h>
  #include <unistd.h>
#endif

namespace oo_alloc {
namespace utils {

// aligns a size/address to the nearest multiple of align
// NOTE: align must be a power of 2.
constexpr inline std::size_t align_up(std::size_t size, std::size_t align) {
  return (size + (align - 1)) & ~(align - 1);
}

// calculates amount of padding bytes required to align a mem address
// NOTE: align must be a power of 2.
inline std::size_t calc_pad(std::uintptr_t ptr, std::size_t align) {
  return static_cast<std::size_t>((align - (ptr & (align - 1))) & (align - 1));
}

// memory page size (minimum init size)
inline std::size_t page_size() {
  static const std::size_t size = []() {
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<std::size_t>(sysInfo.dwPageSize);
#else
    return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
  }();
  return size;
}

// OS call for memory page
inline void* os_alloc(std::size_t size) {
#if defined(_WIN32)
  void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  return ptr;
#else
  void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  return (ptr == MAP_FAILED) ? nullptr : ptr;
#endif
}

// OS call for free
inline void os_free(void* ptr, std::size_t size) {
  if (!ptr) 
    return;

#if defined(_WIN32)
  (void)size;
  VirtualFree(ptr, 0, MEM_RELEASE);
#else
  munmap(ptr, size);
#endif
}

}
}
