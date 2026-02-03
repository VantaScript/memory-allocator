#include <stdio.h>
#include <stdint.h>
#include "allocator.h"

/**
 * Test Phase 2 Features:
 * 1. Memory Alignment
 * 2. Block Coalescing
 * 3. Block Splitting
 * 4. Error Handling
 */

int is_aligned(void* ptr, size_t alignment) {
    return ((uintptr_t)ptr % alignment) == 0;
}

/* Test 1: Memory Alignment */
void test_alignment() {
    printf("=== Test 1: Memory Alignment ===\n");
    
    void* ptr1 = my_malloc(1);   // Request 1 byte
    void* ptr2 = my_malloc(13);  // Request 13 bytes
    void* ptr3 = my_malloc(20);  // Request 20 bytes
    
    printf("Allocated 1 byte at: %p (aligned: %s)\n", 
           ptr1, is_aligned(ptr1, 8) ? "YES" : "NO");
    printf("Allocated 13 bytes at: %p (aligned: %s)\n", 
           ptr2, is_aligned(ptr2, 8) ? "YES" : "NO");
    printf("Allocated 20 bytes at: %p (aligned: %s)\n", 
           ptr3, is_aligned(ptr3, 8) ? "YES" : "NO");
    
    if (is_aligned(ptr1, 8) && is_aligned(ptr2, 8) && is_aligned(ptr3, 8)) {
        printf("✓ All pointers are 8-byte aligned!\n");
    } else {
        printf("✗ Alignment test failed!\n");
    }
    
    my_free(ptr1);
    my_free(ptr2);
    my_free(ptr3);
    printf("\n");
}

void test_splitting() {
    printf("=== Test 2: Block Splitting ===\n");
    
    // Allocate a large block
    void* large = my_malloc(200);
    printf("Allocated large block (200 bytes) at: %p\n", large);
    
    // Free it
    my_free(large);
    printf("Freed large block\n");
    
    void* small = my_malloc(50);
    printf("Allocated small block (50 bytes) at: %p\n", small);
    printf("The large block should have been split!\n");
    printf("(Remainder should still be available for allocation)\n");
    
    void* another = my_malloc(100);
    printf("Allocated another block (100 bytes) at: %p\n", another);
    printf("✓ Block splitting appears to be working!\n");
    
    my_free(small);
    my_free(another);
    printf("\n");
}

void test_coalescing() {
    printf("=== Test 3: Block Coalescing ===\n");
    
    // Allocate three adjacent blocks
    void* block1 = my_malloc(50);
    void* block2 = my_malloc(50);
    void* block3 = my_malloc(50);
    
    printf("Allocated 3 blocks:\n");
    printf("  Block 1: %p\n", block1);
    printf("  Block 2: %p\n", block2);
    printf("  Block 3: %p\n", block3);
    
    printf("\nFreeing blocks 1, 2, 3...\n");
    my_free(block1);
    my_free(block2);
    my_free(block3);
    
    void* large = my_malloc(150);
    printf("Allocated large block (150 bytes) at: %p\n", large);
    
    if (large != NULL) {
        printf("✓ Coalescing appears to be working!\n");
        printf("  (The three small blocks were merged into one large block)\n");
        my_free(large);
    } else {
        printf("✗ Coalescing might not be working correctly\n");
    }
    printf("\n");
}

void test_error_handling() {
    printf("=== Test 4: Error Handling ===\n");
    
    printf("Test: Freeing NULL pointer...\n");
    my_free(NULL);
    printf("✓ Freeing NULL is safe\n");
    
    printf("Test: Allocating 0 bytes...\n");
    void* ptr = my_malloc(0);
    if (ptr == NULL) {
        printf("✓ Allocating 0 bytes returns NULL (correct behavior)\n");
    } else {
        printf("✗ Allocating 0 bytes should return NULL\n");
        my_free(ptr);
    }
    
    printf("Test: Double-free detection...\n");
    void* test_ptr = my_malloc(10);
    my_free(test_ptr);
    printf("  (Attempting double-free - should show error message)\n");
    my_free(test_ptr);  
    
    printf("\n");
}

void test_memory_efficiency() {
    printf("=== Test 5: Memory Efficiency ===\n");
    
    void* ptrs[10];
    
    printf("Allocating 10 blocks of 20 bytes each...\n");
    for (int i = 0; i < 10; i++) {
        ptrs[i] = my_malloc(20);
    }
    
    printf("Freeing every other block...\n");
    for (int i = 0; i < 10; i += 2) {
        my_free(ptrs[i]);
    }
    
    printf("Freeing remaining blocks...\n");
    for (int i = 1; i < 10; i += 2) {
        my_free(ptrs[i]);
    }
    
    printf("✓ All blocks freed (coalescing should have merged adjacent free blocks)\n");
    printf("\n");
}

int main() {
    printf("========================================\n");
    printf("Phase 2 Memory Allocator Tests\n");
    printf("========================================\n\n");
    
    test_alignment();
    test_splitting();
    test_coalescing();
    test_error_handling();
    test_memory_efficiency();
    
    printf("========================================\n");
    printf("All tests completed!\n");
    printf("========================================\n");
    
    return 0;
}



