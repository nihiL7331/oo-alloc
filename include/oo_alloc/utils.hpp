#pragma once
#include <cstddef>
#include <cstdint>

namespace oo_alloc {
namespace utils {

// aligns a size/address to the nearest multiple of align
// NOTE: align must be a power of 2.
constexpr inline std::size_t align_up(std::size_t size, std::uint8_t align) {
  return (size + (align - 1)) & ~(align - 1);
}

// calculates amount of padding bytes required to align a mem address
// NOTE: align must be a power of 2.
inline std::uint8_t calc_pad(std::uintptr_t ptr, std::uint8_t align) {
  return static_cast<std::uint8_t>((align - (ptr & (align - 1))) & (align - 1));
}

}
}
