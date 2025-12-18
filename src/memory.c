/*
 * Brisk Language - Memory Management Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

size_t bytes_allocated = 0;

void* mem_alloc(size_t size) {
    bytes_allocated += size;
    void* ptr = malloc(size);
    if (ptr == NULL && size > 0) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    return ptr;
}

void* mem_realloc(void* ptr, size_t old_size, size_t new_size) {
    bytes_allocated += new_size - old_size;
    
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    
    void* result = realloc(ptr, new_size);
    if (result == NULL) {
        fprintf(stderr, "Fatal: Out of memory\n");
        exit(1);
    }
    return result;
}

void mem_free(void* ptr, size_t size) {
    if (ptr == NULL) return;
    bytes_allocated -= size;
    free(ptr);
}

void mem_print_stats(void) {
    printf("Memory: %zu bytes allocated\n", bytes_allocated);
}
