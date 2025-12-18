/*
 * Brisk Language - Memory Management
 */

#ifndef BRISK_MEMORY_H
#define BRISK_MEMORY_H

#include <stddef.h>

/* Memory allocation tracking */
extern size_t bytes_allocated;

/* Allocation functions */
void* mem_alloc(size_t size);
void* mem_realloc(void* ptr, size_t old_size, size_t new_size);
void mem_free(void* ptr, size_t size);

/* Debug helpers */
void mem_print_stats(void);

#endif /* BRISK_MEMORY_H */
