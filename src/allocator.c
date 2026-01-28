#include "allocator.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/* ============================================================================
 * PHASE 2: Enhanced Memory Allocator with Coalescing, Splitting, and Alignment
 * ============================================================================
 */

/* Memory alignment constant - 8 bytes is standard for most systems
 * This ensures all allocated blocks start at addresses divisible by 8
 * Example: 0x1000, 0x1008, 0x1010 (all divisible by 8)
 */
#define ALIGNMENT 8

/* Minimum block size - blocks smaller than this won't be split
 * This prevents creating tiny fragments that are unusable
 */
#define MIN_BLOCK_SIZE (BLOCK_SIZE + ALIGNMENT)

/* Block structure (header)
 * Each allocated block has this header before the user data
 * 
 * size: Size of user data (not including header/footer)
 * free: 1 if block is free, 0 if allocated
 * next: Pointer to next block in free list (only used when free)
 * 
 * Memory layout:
 * [Header: size, free, next] [User Data] [Footer: size, free]
 */
typedef struct block {
    size_t size;        // Size of user-accessible memory
    int free;           // 1 = free, 0 = allocated
    struct block* next; // Next block in free list (NULL if allocated)
} block_t;

/* Footer structure - stored at the end of each block
 * This allows us to traverse backwards to find previous blocks
 * for coalescing
 */
typedef struct {
    size_t size;  // Same as header size
    int free;     // Same as header free flag
} footer_t;

/* Size of block header */
#define BLOCK_SIZE sizeof(block_t)

/* Size of block footer */
#define FOOTER_SIZE sizeof(footer_t)

/* Total metadata size (header + footer) */
#define METADATA_SIZE (BLOCK_SIZE + FOOTER_SIZE)

/* Global free list - linked list of all free blocks */
static block_t* free_list = NULL;

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

/**
 * Align size to ALIGNMENT boundary
 * 
 * How it works:
 * - (size + ALIGNMENT - 1): Round up
 * - & ~(ALIGNMENT - 1): Clear lower bits to force alignment
 * 
 * Example: size = 13, ALIGNMENT = 8
 * - 13 + 8 - 1 = 20
 * - ~7 = 0xFFFFFFF8 (clears last 3 bits)
 * - 20 & 0xFFFFFFF8 = 16 (aligned!)
 */
static size_t align_size(size_t size) {
    if (size == 0) return ALIGNMENT;  // Minimum size
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/**
 * Get the footer of a block
 * Footer is located at: block_start + header_size + user_data_size
 * 
 * @param block: Pointer to block header
 * @return: Pointer to block footer
 */
static footer_t* get_footer(block_t* block) {
    if (!block) return NULL;
    // Move forward: header + user data = footer location
    return (footer_t*)((char*)block + BLOCK_SIZE + block->size);
}

/**
 * Get the previous block using footer information
 * 
 * How it works:
 * 1. Get footer of current block (which is at end of previous block's data)
 * 2. Move back: footer + footer_size = end of previous block
 * 3. Move back: previous_block_size = start of previous block
 * 
 * @param block: Current block
 * @return: Previous block, or NULL if this is the first block
 */
static block_t* get_previous_block(block_t* block) {
    if (!block) return NULL;
    
    // Get footer of previous block (located just before current block)
    footer_t* prev_footer = (footer_t*)((char*)block - FOOTER_SIZE);
    
    // Check if we're at the start of heap (no previous block)
    if ((char*)prev_footer < (char*)free_list) {
        return NULL;
    }
    
    // Calculate previous block start
    // Previous block ends at: current_block_start - footer_size
    // Previous block starts at: end - prev_block_size - header_size
    block_t* prev_block = (block_t*)((char*)block - prev_footer->size - METADATA_SIZE);
    
    return prev_block;
}

/**
 * Get the next block (physically adjacent in memory)
 * 
 * NOTE: This is simplified - we can't reliably find the next block
 * without tracking heap boundaries. For now, we'll only coalesce
 * with the previous block (using footers).
 * 
 * @param block: Current block
 * @return: Next block, or NULL (simplified implementation)
 */
static block_t* get_next_block(block_t* block) {
    // Simplified: We can't reliably determine next block without heap tracking
    // This would require maintaining a list of all blocks or tracking heap end
    // For Phase 2, we focus on previous block coalescing (which uses footers)
    (void)block;  // Suppress unused parameter warning
    return NULL;
}

/**
 * Set footer information for a block
 * Footer must match header information
 * 
 * @param block: Block to set footer for
 */
static void set_footer(block_t* block) {
    if (!block) return;
    footer_t* footer = get_footer(block);
    footer->size = block->size;
    footer->free = block->free;
}

/**
 * Remove a block from the free list
 * 
 * @param block: Block to remove
 */
static void remove_from_free_list(block_t* block) {
    if (!block || !free_list) return;
    
    // If it's the first block in free list
    if (free_list == block) {
        free_list = block->next;
        block->next = NULL;
        return;
    }
    
    // Find the block before this one in free list
    block_t* current = free_list;
    while (current && current->next != block) {
        current = current->next;
    }
    
    // Remove from list
    if (current) {
        current->next = block->next;
        block->next = NULL;
    }
}

/**
 * Add a block to the free list (at the beginning for simplicity)
 * 
 * @param block: Block to add
 */
static void add_to_free_list(block_t* block) {
    if (!block) return;
    
    block->free = 1;
    block->next = free_list;
    free_list = block;
    set_footer(block);  // Update footer
}

/**
 * Coalesce (merge) current block with adjacent free blocks
 * 
 * This reduces memory fragmentation by merging adjacent free blocks
 * into one larger free block.
 * 
 * @param block: Block to coalesce
 * @return: Pointer to the coalesced block (may be different from input)
 */
static block_t* coalesce(block_t* block) {
    if (!block) return NULL;
    
    // For Phase 2, we focus on coalescing with previous block using footers
    // Full bidirectional coalescing requires tracking heap boundaries (Phase 4+)
    block_t* prev = get_previous_block(block);
    
    if (prev != NULL && prev->free) {
        // Merge with previous block
        // Remove previous from free list (it will be merged)
        remove_from_free_list(prev);
        
        // Merge: previous block size + metadata + current block size
        prev->size += METADATA_SIZE + block->size;
        
        // Update footer of merged block
        set_footer(prev);
        
        // Return the merged block (now starts at prev)
        return prev;
    }
    
    // No coalescing possible - return block as-is
    return block;
}

/**
 * Split a block if it's large enough
 * 
 * If a free block is much larger than requested, we split it:
 * - Use the first part for allocation
 * - Keep the remainder as a free block
 * 
 * @param block: Block to potentially split
 * @param size: Requested size
 * @return: Pointer to block to use (may be split from original)
 */
static block_t* split_block(block_t* block, size_t size) {
    if (!block) return NULL;
    
    // Check if block is large enough to split
    // We need: requested_size + metadata for new block + minimum block size
    size_t total_needed = size + METADATA_SIZE + MIN_BLOCK_SIZE;
    
    if (block->size < total_needed) {
        // Too small to split, use entire block
        return block;
    }
    
    // Split the block
    // Calculate where the new block starts
    block_t* new_block = (block_t*)((char*)block + METADATA_SIZE + size);
    
    // Set up the new (remainder) block
    new_block->size = block->size - size - METADATA_SIZE;
    new_block->free = 1;
    new_block->next = block->next;  // Maintain free list connection
    
    // Update original block
    block->size = size;
    block->free = 0;  // Will be allocated
    set_footer(block);
    
    // Add remainder to free list
    add_to_free_list(new_block);
    
    return block;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * Allocate memory (Phase 2 enhanced version)
 * 
 * Features:
 * - Memory alignment (8-byte boundaries)
 * - Block splitting (use only what's needed)
 * - First-fit allocation strategy
 * - Error handling
 * 
 * @param size: Number of bytes to allocate
 * @return: Pointer to allocated memory, or NULL on error
 */
void* my_malloc(size_t size) {
    // Error handling: Check for invalid size
    if (size == 0) {
        return NULL;  // Standard malloc returns NULL for size 0
    }
    
    // Align the requested size
    size_t aligned_size = align_size(size);
    
    block_t* current = free_list;
    block_t* prev = NULL;
    
    // First-fit search: Find first free block large enough
    while (current) {
        if (current->free && current->size >= aligned_size) {
            // Found a suitable block!
            
            // Remove from free list (will be allocated)
            remove_from_free_list(current);
            
            // Split if block is much larger than needed
            current = split_block(current, aligned_size);
            
            // Mark as allocated
            current->free = 0;
            set_footer(current);
            
            // Return pointer to user data (after header)
            return (void*)(current + 1);
        }
        prev = current;
        current = current->next;
    }
    
    // No suitable free block found - allocate new memory from OS
    // Calculate total size needed: header + aligned data + footer
    size_t total_size = METADATA_SIZE + aligned_size;
    
    // Request memory from OS using sbrk (system break)
    void* mem = sbrk(total_size);
    if (mem == (void*)-1) {
        // sbrk failed - out of memory
        return NULL;
    }
    
    // Initialize new block
    block_t* new_block = (block_t*)mem;
    new_block->size = aligned_size;
    new_block->free = 0;  // This block is allocated
    new_block->next = NULL;  // Not in free list (it's allocated)
    
    // Set footer (needed for coalescing when this block is freed)
    set_footer(new_block);
    
    // Note: We don't add allocated blocks to the free list
    // When freed, my_free() will add it and handle coalescing
    
    // Return pointer to user data (skip over header)
    return (void*)(new_block + 1);
}

/**
 * Free previously allocated memory (Phase 2 enhanced version)
 * 
 * Features:
 * - Block coalescing (merge adjacent free blocks)
 * - Double-free detection
 * - Error handling
 * 
 * @param ptr: Pointer to memory to free (must be from my_malloc)
 */
void my_free(void* ptr) {
    // Error handling: Check for NULL pointer
    if (!ptr) {
        return;  // Freeing NULL is safe (like standard free)
    }
    
    // Get block header (it's located just before user data)
    block_t* block = ((block_t*)ptr) - 1;
    
    // Error handling: Check for double-free
    if (block->free) {
        // Block is already free - this is a double-free error!
        fprintf(stderr, "Error: Double-free detected at %p\n", ptr);
        return;
    }
    
    // Error handling: Basic sanity check
    if (block->size == 0 || block->size > 1024 * 1024 * 1024) {  // 1GB sanity check
        fprintf(stderr, "Error: Invalid block size detected\n");
        return;
    }
    
    // Mark as free
    block->free = 1;
    set_footer(block);
    
    // Coalesce with adjacent free blocks
    block = coalesce(block);
    
    // Add to free list
    add_to_free_list(block);
}
