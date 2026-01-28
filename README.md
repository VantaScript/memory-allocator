# Custom Memory Allocator in C 

**A custom dynamic memory allocator that mimics the behavior of `malloc` and `free`.**

---

## Overview
This project demonstrates low-level memory management in C.  
It implements a simple **free-list based memory allocator** to allocate and free memory blocks manually without using the standard `malloc` and `free`.  

It’s designed for learning how heap memory works and how dynamic memory allocation is implemented under the hood.

---

## Project Structure

```
memory-allocator/
├─ src/
│  ├─ allocator.c     # implementation of my_malloc & my_free
│  └─ test.c          # test program demonstrating allocator
├─ include/
│  └─ allocator.h     # function declarations
├─ Makefile           # build instructions
└─ README.md

```
---

## Features
	•	my_malloc(size_t size) — allocate memory blocks
	•	my_free(void* ptr) — free previously allocated blocks
	•	Tracks allocated and free memory using a free-list
	•	Lightweight, modular, and educational

 ---


## Learning Goals
	•	Understand heap memory and dynamic memory allocation
	•	Learn pointer arithmetic and memory block layout
	•	Implement and manage a linked list for free memory blocks
	•	Explore how malloc and free work internally

---

## Tech Used
	•	Language: C (C99)
	•	Platform: Linux / macOS (POSIX)

---

## Roadmap

### ✅ Phase 1: Core Implementation (Current)
- [x] Basic `my_malloc()` implementation
- [x] Basic `my_free()` implementation
- [x] Free-list data structure
- [x] First-fit allocation strategy
- [x] Simple test program

### 🔄 Phase 2: Memory Management Improvements (In Progress)
- [ ] **Block Coalescing**: Merge adjacent free blocks to reduce fragmentation
- [ ] **Block Splitting**: Split large free blocks to better utilize memory
- [ ] **Memory Alignment**: Ensure allocated blocks are properly aligned (e.g., 8-byte alignment)
- [ ] **Boundary Tags**: Add footer tags to enable backward traversal for coalescing
- [ ] **Error Handling**: Improve error handling and edge case management

### 📋 Phase 3: Additional Standard Functions
- [ ] **my_calloc()**: Implement zero-initialized memory allocation
- [ ] **my_realloc()**: Implement memory reallocation with size adjustment
- [ ] **my_memalign()**: Implement aligned memory allocation
- [ ] **Memory Statistics**: Add functions to track allocation statistics (total allocated, free blocks, etc.)

### 🚀 Phase 4: Advanced Allocation Strategies
- [ ] **Best-fit Algorithm**: Implement best-fit allocation strategy
- [ ] **Worst-fit Algorithm**: Implement worst-fit allocation strategy
- [ ] **Buddy System**: Implement buddy allocator for power-of-2 sized blocks
- [ ] **Segregated Free Lists**: Maintain separate free lists for different size classes
- [ ] **Thread Safety**: Add mutex locks for multi-threaded environments

### 🧪 Phase 5: Testing & Validation
- [ ] **Comprehensive Test Suite**: Create extensive unit tests
- [ ] **Memory Leak Detection**: Implement leak detection mechanisms
- [ ] **Stress Testing**: Test with various allocation patterns
- [ ] **Performance Benchmarking**: Compare performance with standard malloc/free
- [ ] **Valgrind Compatibility**: Ensure compatibility with memory debugging tools

### 🔧 Phase 6: Optimization & Polish
- [ ] **Performance Optimization**: Optimize allocation and deallocation speed
- [ ] **Memory Pool**: Implement memory pools for frequently allocated sizes
- [ ] **Debug Mode**: Add debug mode with detailed logging and validation
- [ ] **Documentation**: Expand code documentation and add usage examples
- [ ] **CI/CD**: Set up continuous integration for automated testing

### 🎯 Phase 7: Advanced Features (Future)
- [ ] **Garbage Collection**: Optional garbage collection for automatic memory management
- [ ] **Memory Profiling**: Built-in profiling tools for memory usage analysis
- [ ] **Custom Allocation Policies**: Allow users to configure allocation strategies
- [ ] **Platform-Specific Optimizations**: Optimize for specific architectures
- [ ] **Memory Defragmentation**: Implement periodic defragmentation routines

---

## License
This project is licensed under the MIT License — feel free to use and modify for learning purposes.
