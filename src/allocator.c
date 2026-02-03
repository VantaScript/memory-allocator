#include "allocator.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ALIGNMENT 8
#define MIN_BLOCK_SIZE (BLOCK_SIZE + ALIGNMENT)

typedef struct block {
  size_t size;
  int free;
  struct block *next;
} block_t;

typedef struct {
  size_t size;
  int free;
} footer_t;

#define BLOCK_SIZE sizeof(block_t)
#define FOOTER_SIZE sizeof(footer_t)
#define METADATA_SIZE (BLOCK_SIZE + FOOTER_SIZE)

static block_t *free_list = NULL;

typedef struct {
  size_t total_allocated;
  size_t total_freed;
  size_t peak_allocated;
  size_t num_allocations;
  size_t num_frees;
  size_t num_free_blocks;
  size_t total_free_space;
} allocator_stats_t;

static allocator_stats_t stats = {0};

static void update_stats_after_alloc(size_t size) {
  stats.total_allocated += size;
  stats.num_allocations++;
  if (stats.total_allocated > stats.peak_allocated) {
    stats.peak_allocated = stats.total_allocated;
  }
}

static void update_stats_after_free(size_t size) {
  stats.total_allocated -= size;
  stats.total_freed += size;
  stats.num_frees++;
}

static void update_free_list_stats(void) {
  stats.num_free_blocks = 0;
  stats.total_free_space = 0;

  block_t *current = free_list;
  while (current) {
    stats.num_free_blocks++;
    stats.total_free_space += current->size;
    current = current->next;
  }
}

static size_t align_size(size_t size) {
  if (size == 0)
    return ALIGNMENT;
  return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static footer_t *get_footer(block_t *block) {
  if (!block)
    return NULL;
  return (footer_t *)((char *)block + BLOCK_SIZE + block->size);
}

static block_t *get_previous_block(block_t *block) {
  if (!block)
    return NULL;

  footer_t *prev_footer = (footer_t *)((char *)block - FOOTER_SIZE);

  if ((char *)prev_footer < (char *)free_list) {
    return NULL;
  }

  block_t *prev_block =
      (block_t *)((char *)block - prev_footer->size - METADATA_SIZE);

  return prev_block;
}

__attribute__((unused)) static block_t *get_next_block(block_t *block) {
  (void)block;
  return NULL;
}

static void set_footer(block_t *block) {
  if (!block)
    return;
  footer_t *footer = get_footer(block);
  footer->size = block->size;
  footer->free = block->free;
}

static void remove_from_free_list(block_t *block) {
  if (!block || !free_list)
    return;

  if (free_list == block) {
    free_list = block->next;
    block->next = NULL;
    return;
  }

  block_t *current = free_list;
  while (current && current->next != block) {
    current = current->next;
  }

  if (current) {
    current->next = block->next;
    block->next = NULL;
  }
}

static void add_to_free_list(block_t *block) {
  if (!block)
    return;

  block->free = 1;
  block->next = free_list;
  free_list = block;
  set_footer(block);
}

static block_t *coalesce(block_t *block) {
  if (!block)
    return NULL;

  block_t *prev = get_previous_block(block);

  if (prev != NULL && prev->free) {
    remove_from_free_list(prev);
    prev->size += METADATA_SIZE + block->size;
    set_footer(prev);
    return prev;
  }

  return block;
}

static block_t *split_block(block_t *block, size_t size) {
  if (!block)
    return NULL;

  size_t total_needed = size + METADATA_SIZE + MIN_BLOCK_SIZE;

  if (block->size < total_needed) {
    return block;
  }

  block_t *new_block = (block_t *)((char *)block + METADATA_SIZE + size);

  new_block->size = block->size - size - METADATA_SIZE;
  new_block->free = 1;
  new_block->next = block->next;

  block->size = size;
  block->free = 0;
  set_footer(block);

  add_to_free_list(new_block);

  return block;
}

void *my_malloc(size_t size) {
  if (size == 0) {
    return NULL;
  }

  size_t aligned_size = align_size(size);

  block_t *current = free_list;

  while (current) {
    if (current->free && current->size >= aligned_size) {
      remove_from_free_list(current);
      current = split_block(current, aligned_size);

      current->free = 0;
      set_footer(current);

      update_stats_after_alloc(current->size);
      update_free_list_stats();

      return (void *)(current + 1);
    }
    current = current->next;
  }

  size_t total_size = METADATA_SIZE + aligned_size;

  void *mem = sbrk(total_size);
  if (mem == (void *)-1) {
    return NULL;
  }

  block_t *new_block = (block_t *)mem;
  new_block->size = aligned_size;
  new_block->free = 0;
  new_block->next = NULL;

  set_footer(new_block);

  update_stats_after_alloc(new_block->size);
  update_free_list_stats();

  return (void *)(new_block + 1);
}

void my_free(void *ptr) {
  if (!ptr) {
    return;
  }

  block_t *block = ((block_t *)ptr) - 1;

  if (block->free) {
    fprintf(stderr, "Error: Double-free detected at %p\n", ptr);
    return;
  }

  if (block->size == 0 || block->size > 1024 * 1024 * 1024) {
    fprintf(stderr, "Error: Invalid block size detected\n");
    return;
  }

  block->free = 1;
  set_footer(block);

  update_stats_after_free(block->size);

  block = coalesce(block);

  add_to_free_list(block);

  update_free_list_stats();
}

void *my_calloc(size_t num, size_t size) {
  if (num == 0 || size == 0) {
    return NULL;
  }

  size_t total_size = num * size;
  if (total_size / num != size) {
    return NULL;
  }

  void *ptr = my_malloc(total_size);

  if (!ptr) {
    return NULL;
  }

  memset(ptr, 0, total_size);

  return ptr;
}

void *my_realloc(void *ptr, size_t new_size) {
  if (!ptr) {
    return my_malloc(new_size);
  }

  if (new_size == 0) {
    my_free(ptr);
    return NULL;
  }

  block_t *block = ((block_t *)ptr) - 1;

  if (block->free) {
    fprintf(stderr, "Error: realloc on freed block\n");
    return NULL;
  }

  size_t aligned_new_size = align_size(new_size);

  if (block->size == aligned_new_size) {
    return ptr;
  }

  if (aligned_new_size < block->size) {
    return ptr;
  }

  void *new_ptr = my_malloc(new_size);
  if (!new_ptr) {
    return NULL;
  }

  size_t copy_size =
      block->size < aligned_new_size ? block->size : aligned_new_size;
  memcpy(new_ptr, ptr, copy_size);

  my_free(ptr);

  return new_ptr;
}

size_t my_get_total_allocated(void) { return stats.total_allocated; }

size_t my_get_total_freed(void) { return stats.total_freed; }

size_t my_get_peak_allocated(void) { return stats.peak_allocated; }

size_t my_get_num_allocations(void) { return stats.num_allocations; }

size_t my_get_num_frees(void) { return stats.num_frees; }

size_t my_get_num_free_blocks(void) {
  update_free_list_stats();
  return stats.num_free_blocks;
}

size_t my_get_total_free_space(void) {
  update_free_list_stats();
  return stats.total_free_space;
}
