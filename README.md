# oo-alloc

A collection of custom C++ allocators.

## Build & run
```bash
mkdir build && cd build
cmake ..

# build the static library (.a)
cmake --build . --config Release

# build and run performance benchmarks
cmake --build . --target run-benchmarks

# build and run individual tests
cmake  --build . --target test-arena 
# available: test-arena, test-pool, test-stack, test-free, test-track, test-rbtree, test-tree, test-all
```

## Introduction

Custom allocators are tightly coupled with certain data structures, access patterns and object lifetimes. 
Just like choosing the right data structure, picking the correct allocation strategy can grant noticeable performance gains.
Each allocator comes with its own quirks: different time complexities, memory overheads and optional constraints.
It is crucial to pick the right tool for the job. 
The overview available below can help you do exactly that.

## Overview

| Type             | `alloc`    | `free`     | `clear`   | `realloc`      | Overhead | Best for          | Constraints             |
| :---             | :---:      | :---:      | :---:     | :---:          | :---:    | :---              | :---                    |
| **Arena**        | `O(1)`     | *N/A*      | `O(1)`    | `O(1)`*/`O(n)` | 0B       | Bulk allocations  | No individual frees     |
| **Stack**        | `O(1)`     | `O(1)`*    | `O(1)`    | `O(1)`*/`O(n)` | ~8B      | Temporary data    | Strict LIFO             |
| **Pool**         | `O(1)`     | `O(1)`     | `O(1)`    | *N/A*          | 0B       | Identical objects | Fixed sizes             |
| **Free List**    | `O(n)`     | `O(n)`     | `O(1)`    | `O(n)`         | ~16B     | General purpose   | Slow search / coalesce  |
| **Free Tree**    | `O(log n)` | `O(log n)` | `O(1)`    | `O(log n)`     | ~32B     | General purpose   | High block overhead     |
| **Segregated**** | `O(1)`     | `O(1)`     | `O(1)`    | `O(1)`         | ~16B     | General purpose   | Complex to tune         |
| **Buddy****      | `O(1)`     | `O(1)`     | `O(1)`    | `O(1)`         | 0B       | OS memory         | Fragmentation (in)      |
| **Slab****       | `O(1)`     | `O(1)`     | `O(1)`    | *N/A*          | 0B       | Object caching    | Single type             |

<sub>\*You can only operate on the top-most allocation.</sub><br>
<sub>\*\*Not yet available in this repository (in development).</sub>

## Implementation

### Arena (linear) allocator

The simplest allocator.
It keeps a pointer to the starting address of the large, contiguous block allocated on `init`.
Also stores an `m_offset` integer, which holds the relative position of the last allocated memory from the pointer.

Each time `alloc` is called, size of said allocation is added to the `m_offset`.
Minimal fragmentation is achieved due to the sequential allocation - the only space wasted is used for alignment.

#### Internal structure

```cpp
class ArenaAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_total_size;
  std::size_t m_offset;

public:
  void* alloc(std::size_t size, std::size_t align);
  bool  init(std::size_t size);
  void  clear();
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align);
  std::size_t capacity() const;
};
```

When `alloc` is called, it calculates the memory alignment, adds the requested `size` plus the alignment to the `m_offset`, and returns the previous address.
This makes `alloc` an instant *O(1)* operation.

Because it only tracks a single forward-moving offset, you can't free individual objects.
You can only clear the entire arena, which is done by resetting `m_offset` to zero, resulting in a *O(1)* `clear` time complexity.

When `realloc` is called, it can only expand a block in-place if the pointer belongs to the most recently allocated object. 
If `realloc` is called on an older block, the allocator must allocate a brand new chunk of memory at the current offset and copy the data. 
The arena does not support individiual freeing, so the old memory block is left behind as dead, wasted space.

<p align="center">
  <img src="docs/assets/arena_active.svg" alt="arena allocator active state">
  <br>
  <em><sub>Arena allocator after two allocations. The offset points to the start of the free space.</sub></em>
</p>

<p align="center">
  <img src="docs/assets/arena_clear.svg" alt="arena allocator after clear">
  <br>
  <em><sub>Arena allocator after clear() is called. It holds no data.</sub></em>
</p>

### Stack allocator

As the name suggests, it treats the memory as a stack. 
It builds upon the Arena allocator. 
Contrary to the Arena, it supports individual `free` operations, but due to the strict Last-In First-Out (LIFO) restrictions, you can only "pop" the most recently allocated element. 

Its definition is practically identical to the arena allocator, but it has an additional method `free`.
What differs is that each allocated block has a header before it, which stores the previous `m_offset`.


#### Internal structure

```cpp
class StackAllocator {
private:
  void*       m_start_ptr;
  std::size_t m_offset;
  std::size_t m_total_size;

public:
  void* alloc(std::size_t size, std::size_t align);
  void  free(void* ptr);
  bool  init(std::size_t size);
  void  clear();
  std::size_t capacity() const;
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align);
};
```

When `alloc` is called, the allocator calculates the required padding for alignment, reserves space for both the header and the requested `size`, and writes the previous `m_offset` into the header just behind the returned pointer. This makes `alloc` an instant *O(1)* operation.

On `free`, the allocator simply reads the header immediately before the given pointer and restores `m_offset` to the value stored in it, hence deallocating the block. Since we don't need to search for the data, it's also an *O(1)* operation.

When `realloc` is called, similarly to the arena, it expands in-place if the pointer is the top-most allocation on stack. 
If expanding an older block, it allocates new memory at the top and copies the data.
The allocator doesn't free the old memory block in this scenario, since calling `free` on an older block would destroy the newly allocated memory.

<p align="center">
  <img src="docs/assets/stack_active.svg" alt="stack allocator active state">
  <br>
  <em><sub>Stack allocator after two allocations. The hidden headers (h1, h2) are placed immediately before the user data.</sub></em>
</p>

<p align="center">
  <img src="docs/assets/stack_free.svg" alt="stack allocator after free">
  <br>
  <em><sub>After calling free() on data 2, the allocator reads h2 and instantly snaps m_offset back to the end of data 1.</sub></em>
</p>

### Pool allocator

It's a very limited, but exceptionally fast and simple allocator.
It works by dividing up its entire memory block into equally sized segments, reffered to as **chunks**.

To track free memory, it uses a singly linked list.
To achieve zero memory overhead, it uses a intrusive list. 
It stores the "next" pointer directly inside the free chunks themselves. 
When a chunk is free, it acts as a list node. 
When it is allocated, that pointer is overwritten by the users data.

#### Internal structure

```cpp
class PoolAllocator {
private:
  std::size_t m_chunk_size;
  std::size_t m_chunk_align;
  void*       m_start_ptr;
  std::size_t m_total_size;
  void*       m_free_list_head;

public:
  void* alloc(std::size_t size, std::size_t align);
  void  free(void* ptr);
  bool  init(std::size_t size);
  void  clear();
  std::size_t capacity() const;
};
```

When `alloc` is called, it pops the `m_free_list_head`, updates the head to point to the next free chunk in the list, and returns the popped address.
Since there is no list traversal required, this is an instant *O(1)* operation.

On `free`, it casts the passed `ptr` into a list node, sets its "next" pointer to the current `m_free_list_head`, and then updates `m_free_list_head` to point to `ptr`. This frees the chunk in *O(1)* time.

Because the pool operates on fixed-size chunks, `realloc` is impossible.

<p align="center">
  <img src="docs/assets/pool_active.svg" alt="pool allocator active state">
  <br>
  <em><sub>Pool allocator in a fragmented state. The free chunks store "next" pointers directly inside their empty space.</sub></em>
</p>

<p align="center">
  <img src="docs/assets/pool_free.svg" alt="pool allocator after free">
  <br>
  <em><sub>After calling free() on data 1, it gets pushed to the front of the intrusive linked list. The head pointer is updated in O(1) time.</sub></em>
</p>

### Free list allocator

It's a specific type of general-purpose dynamic allocator, widely used under the hood for heap memory management. General-purpose allocators gained popularity due to having essentially no restrictions on allocation size or order. However, their performance varies greatly based on the underlying data structure used to track the free blocks:
- **Free List**: Achieves *O(n)* time complexity using a singly-linked or doubly-linked list.
- **Free Tree**: Achieves *O(log n)* time complexity using a bin search tree (e.g, Red-black tree).
- **Segregated Fits**: Achieves (mostly) *O(1)* time complexity using an array of size-segregated free lists (buckets).

The implementation below details the **singly-linked free list**. 
The list is sorted by memory address (ascending), and is intrusive, just like in the Pool allocator. 
What differs is that compared to the Pool allocator, the free block stores more data. 
It stores the `size` of the free data, and the address to the `next` free block.

When a chunk of memory is allocated, it is prefixed with a `AllocHeader`, which holds the `size` and `pad`, required to align the data.

#### Internal structure

```cpp
class FreeListAllocator {
private:
  struct FreeBlock {
    std::size_t size;
    FreeBlock *next;
  };
  struct AllocHeader {
    std::size_t size;
    std::size_t pad;
  };
  void*       m_start_ptr;
  std::size_t m_total_size;
  FreeBlock*  m_free_list_head;

public:
  void* alloc(std::size_t size, std::size_t align);
  void  free(void* ptr);
  bool  init(std::size_t size);
  void  clear();
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align);
  std::size_t capacity() const;
};
```
<sub>For clarity purposes, the helpers/handlers are not shown above. You can find the full definition [here](include/oo_alloc/FreeListAllocator.hpp)</sub>

When `alloc` is called, it traverses through the list of free blocks, picking one matching the wanted size the closest (**best-fit**), or the first that can contain the memory to allocate (**first-fit**), and allocates the data with the hidden header before it. 
Due to the required list traversal, the time complexity is *O(n)*.

When `free` is called, reads the `size` and `pad` from the hidden header, and casts the memory back into a `FreeBlock`.
To prevent external fragmentation, it then **coalesces** neighboring free memory blocks.

Coalescence is achieved by checking if the end of one free block perfectly touches the start of the next. 
If they touch, their sizes are summed together, and the second block is physically removed from the linked list. 
To prevent pointer invalidation bugs, it is best practice to always coalesce a block with its next neighbor before coalescing with its previous neighbor. 
Because finding the correct insertion point in the address-sorted list requires traversal, `free` also carries an *O(n)* time complexity.

When `realloc` is called, it first checks if the block can be shrunk in-place. 
If expanding, it checks adjacent memory. 
It will attempt to (in that order): expand right, expand left (which requires shifting the data via `std::memmove`), and expand in both directions simultaneously.
Only if all neighboring blocks are occupied or too small will it fall back to a standard `alloc`->`copy`->`free` cycle.

<p align="center">
  <img src="docs/assets/free_active.svg" alt="free list allocator active state">
  <br>
  <em><sub>Free list in a highly fragmented state.</sub></em>
</p>

<p align="center">
  <img src="docs/assets/free_split.svg" alt="free list allocator block splitting">
  <br>
  <em><sub>After calling alloc() for 20B, the allocator uses "first-fit" to slice 50B (header + data 3) off free 1. The remainder of free 1 shrinks to 70B.</sub></em>
</p>

<p align="center">
  <img src="docs/assets/free_coal.svg" alt="free list allocator coalescing">
  <br>
  <em><sub>After calling free() on data 2, the allocator detects that free 2, the newly freed block, and free 3 are adjacent. It coalesces them into a single 250B block.</sub></em>
</p>

### Tracking allocator

Unlike the previous allocators, the Tracking allocator does not manage memory directly. Instead, it acts as a wrapper around any other existing allocator. Its primary purpose is debugging. 

It intercepts calls to `alloc` and `free`, records memory metrics, and then forwards the request to the allocator. It is a great tool for profiling memory usage, detecting leaks, and measuring peak memory consumption during app runtime.

#### Internal structure

```cpp
class TrackingAllocator {
private:
  IAllocator& m_base_allocator;

  std::unordered_map<void *, std::size_t> m_active_allocs;
  std::size_t m_curr_alloced_bytes = 0;
  std::size_t m_peak_alloced_bytes = 0;

public:
  TrackingAllocator(IAllocator& base_allocator);

  void* alloc(std::size_t size, std::size_t align);
  void  free(void* ptr);
  bool  init(std::size_t size);
  void  clear();
  void* realloc(void* ptr, std::size_t old_size, std::size_t new_size, std::size_t align);
  std::size_t capacity() const;

  std::size_t curr_bytes() const { return m_curr_alloced_bytes; }
  std::size_t peak_bytes() const { return m_peak_alloced_bytes; }
  std::size_t active_allocs() const { return m_active_allocs.size(); }
};
```

When `alloc` is called, it increments its internal counters, calculates if a new `m_peak_alloced_bytes` has been reached, and then simply returns the result of `m_base_allocator->alloc()`.

When `free` is called, it decrements `m_curr_alloced_bytes` and forwards the pointer to `m_base_allocator->free()`. Because it only performs basic arithmetic before delegating the actual work, the overhead is extremely minimal, maintaining the time complexity of the underlying allocator.

When `realloc` is called, it acts as a mediator. 
It first verifies the old pointer exists in its tracking map, temporarily untracks it, and forwards the request to the `m_base_allocator`. 
If the base allocator successfully reallocates the memory, it tracks the new pointer and updates the memory metrics. 
If it fails, it rolls back its internal state so the original tracking data is intact.

## Roadmap

* [ ] Implement a size segregated free list allocator.
* [ ] Implement a pow-of-two buddy allocator.
* [ ] Benchmark cache misses via perf.
* [ ] Implement a slab allocator.
* [ ] Add a benchmark/performance README section.
* [ ] Move benchmarks to google/benchmark.
* [ ] Move from hand-written tests to proper stress testing.
* [x] Add realloc description in README.
* [x] Refactor and clean up API.
* [x] Move free-list coalescing to a helper function.
* [x] Implement a red-black tree free list allocator.
* [x] Move from malloc to mmap/VirtualAlloc.
* [x] Add realloc functionality.
* [x] Add a build README section.
* [x] Write a description for each allocator.
* [x] Implement a explicit free-list allocator.

## Sources

* [Linear Allocator from Nicholas Frechette's Blog](https://nfrechette.github.io/2015/05/21/linear_allocator/)
* [Writing My Own Malloc in C by Tsoding](https://youtu.be/sZ8GJ1TiMdk?si=j0XnG7UxBTVji-NJ)
* [mtrebi's memory-allocators repository](https://github.com/mtrebi/memory-allocators)
* [gingerBill's Memory Allocation Strategies - Article Series](https://www.gingerbill.org/series/memory-allocation-strategies/)
* [CS107 - Explicit Free List Allocator, Stanford University](https://web.stanford.edu/class/archive/cs/cs107/cs107.1246/lectures/24/Lecture24.pdf)
* [MallocInternals from glibc wiki](https://sourceware.org/glibc/wiki/MallocInternals)
* [Slab allocation Wikipedia page](https://en.wikipedia.org/wiki/Slab_allocation)
* [Red-black tree Wikipedia page](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree)
* [Data Structures and Algorithms - Red-black trees, University of Michigan](https://www.eecs.umich.edu/courses/eecs380/ALG/red_black.html)
* [Red-Black Trees chapter from Introduction to Algorithms by Cormen, Leiserson, Rivest and Stein](https://www.cs.mcgill.ca/~akroit/math/compsci/Cormen%20Introduction%20to%20Algorithms.pdf)
* [CS0330 - Malloc, Brown University](https://cs.brown.edu/courses/cs033/docs/proj/malloc.pdf)
