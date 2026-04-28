# oo-alloc

A collection of custom C++ allocators.

## Introduction

Custom allocators are tightly coupled with certain 
data structures, access patterns and object lifetimes. 
Just like choosing the right data structure,
picking the correct allocation strategy can grant noticeable performance gains.
Each allocator comes with its own quirks: 
different time complexities, memory overheads and optional constraints.
It is crucial to pick the right tool for the job. 
The overview available below can help you do exactly that.

## Overview

| Type          | `alloc`  | `free`   | `clear` | `realloc` | Overhead | Best for          | Constraints         |
| :---          | :---:    | :---:    | :---:   | :---:     | :---:    | :---              | :---                |
| **Arena**     | $O(1)$   | N/A      | $O(1)$  | $O(1)$*   | OB       | Bulk allocations  | No individual frees |
| **Stack**     | $O(1)$   | $O(1)$*  | $O(1)$  | N/A       | ~8B      | Temporary data    | Strict LIFO         |
| **Pool**      | $O(1)$   | $O(1)$   | $O(1)$  | N/A       | 0B       | Identical objects | Fixed sizes         |
| **Free List** | $O(n)$** | $O(n)$** | $O(1)$  | $O(n)$**  | 16B+     | General purpose   | Fragmentation (in)  |
| *Buddy****    | $O(1)$   | $O(1)$   | $O(1)$  | $O(1)$    | 0B       | OS memory         | Fragmentation (ex)  |
| *Slab****     | $O(1)$   | $O(1)$   | $O(1)$  | N/A       | OB       | Object caching    | Single type         |

<sub>\*You can only operate on the top-most allocation.</sub><br>
<sub>\*\*Using a size segregated free-list (size buckets) implementation, it's possible to achieve O(1) time complexity.</sub><br>
<sub>\*\*\*Not yet available in this repository (in development).</sub>

## Implementation

### Arena (linear) allocator

The simplest allocator. \
It keeps a pointer to the starting address of the large, contiguous block allocated on `init`. \
Also stores an `m_offset` integer, which holds the relative position of the last allocated memory from the pointer. \
Each time `alloc` is called, size of said allocation is added to the `m_offset`. \
Minimal fragmentation is achieved due to the sequential allocation - the only space wasted is used for alignment.

#### Internal structure

```cpp
class ArenaAllocator {
private:
  void*  m_start_ptr;
  std::size_t m_total_size;
  std::size_t m_offset;

public:
  void* alloc(std::size_t size, std::uint8_t align);
  bool  init(std::size_t size);
  void clear();
};
```

When `alloc` is called, it calculates the memory alignment, adds the requested `size` plus the alignment to the `m_offset`, and returns the previous address. \
This makes `alloc` an instant $O(1)$ operation. \
Because it only tracks a single forward-moving offset, you can't free individual objects. \
You can only clear the entire arena, which is done by resetting `m_offset` to zero, resulting in a $O(1)$ `clear` time complexity.

## Roadmap

* [x] Implement a explicit free-list allocator.
* [ ] Add realloc functionality.
* [ ] Move free-list allocator to a size buckets implementation.
* [ ] Implement a pow-of-two buddy allocator.
* [ ] Write a description for each allocator.
* [ ] Benchmark cache misses via perf.
* [ ] Implement a slab allocator.

## Sources

* [Linear Allocator from Nicholas Frechette's Blog](https://nfrechette.github.io/2015/05/21/linear_allocator/)
* [Writing My Own Malloc in C by Tsoding](https://youtu.be/sZ8GJ1TiMdk?si=j0XnG7UxBTVji-NJ)
* [mtrebi's memory-allocators repository](https://github.com/mtrebi/memory-allocators)
* [gingerBill's Memory Allocation Strategies - Article Series](https://www.gingerbill.org/series/memory-allocation-strategies/)
* [CS107 - Explicit Free List Allocator, Stanford University](https://web.stanford.edu/class/archive/cs/cs107/cs107.1246/lectures/24/Lecture24.pdf)
* [MallocInternals from glibc wiki](https://sourceware.org/glibc/wiki/MallocInternals)
* [Slab allocation Wikipedia page](https://en.wikipedia.org/wiki/Slab_allocation)
