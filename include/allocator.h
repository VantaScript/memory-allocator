#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

void* my_malloc(size_t size);
void my_free(void* ptr);
void* my_calloc(size_t num, size_t size);
void* my_realloc(void* ptr, size_t new_size);

size_t my_get_total_allocated(void);
size_t my_get_total_freed(void);
size_t my_get_peak_allocated(void);
size_t my_get_num_allocations(void);
size_t my_get_num_frees(void);
size_t my_get_num_free_blocks(void);
size_t my_get_total_free_space(void);

#endif
