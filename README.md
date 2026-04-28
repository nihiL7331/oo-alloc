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

| Type          | `alloc`  | `free`   | `clear` | `realloc` | Overhead | Best for | Constraints |
| :---          | :---:    | :---:    | :---:   | :---:     | :---:    | :---     | :---        |
| **Arena**     | $O(1)$   | N/A      | $O(1)$  | $O(1)$*   | OB       | ...      | ...         |
| **Stack**     | $O(1)$   | $O(1)$*  | $O(1)$  | N/A       | ~8B      | ...      | ...         |
| **Pool**      | $O(1)$   | $O(1)$   | $O(1)$  | N/A       | 0B       | ...      | ...         |
| **Free List** | $O(n)$** | $O(n)$** | $O(1)$  | $O(n)$**  | 16B+     | ...      | ...         |
| *Buddy****    | $O(1)$   | $O(1)$   | $O(1)$  | $O(1)$    | 0B       | ...      | ...         |
| *Slab****     | $O(1)$   | $O(1)$   | $O(1)$  | N/A       | OB       | ...      | ...         |

<sub>\*You can only operate on the top-most allocation.</sub><br>
<sub>\*\*Using a size segregated free-list (size buckets) implementation, it's possible to achieve O(1) time complexity.</sub><br>
<sub>\*\*\*Not yet available in this repository (in development).</sub>

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
