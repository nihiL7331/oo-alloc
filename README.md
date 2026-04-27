# oo-alloc

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
