#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define NULL 0

#define MEMORY_POOL_SIZE 1024 * 1024  // 1MB
static uint8_t kernel_memory_pool[MEMORY_POOL_SIZE];
static size_t kernel_memory_offset = 0;

// Simple structure to keep track of allocated blocks
struct mem_block {
    size_t size; // Size of the block
    struct mem_block* next; // Pointer to the next block
};

// Pointer to the head of the free list
static struct mem_block* free_list = NULL;

// Function to initialize the memory pool
void memory_init() {
    free_list = (struct mem_block*)kernel_memory_pool; // Set the free list to the start of the pool
    free_list->size = MEMORY_POOL_SIZE; // Set the size of the entire pool
    free_list->next = NULL; // No next block
}

// Allocate memory
void* malloc(size_t size) {
    // Align size to 4-byte boundary
    size = (size + 3) & ~3;

    struct mem_block* current = free_list;
    struct mem_block* previous = NULL;

    // Find a suitable block
    while (current) {
        if (current->size >= size) {
            // If the block is larger than needed, split it
            if (current->size > size + sizeof(struct mem_block)) {
                struct mem_block* new_block = (struct mem_block*)((uint8_t*)current + sizeof(struct mem_block) + size);
                new_block->size = current->size - size - sizeof(struct mem_block);
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            }
            // Remove the allocated block from the free list
            if (previous) {
                previous->next = current->next;
            } else {
                free_list = current->next; // Update head of the free list
            }
            return (uint8_t*)current + sizeof(struct mem_block); // Return pointer to the allocated memory
        }
        previous = current;
        current = current->next;
    }
    return NULL; // Out of memory
}

// Free allocated memory
void free(void* ptr) {
    if (!ptr) return; // Do nothing if ptr is NULL

    struct mem_block* block = (struct mem_block*)((uint8_t*)ptr - sizeof(struct mem_block));
    block->next = free_list; // Add the block back to the free list
    free_list = block; // Update head of the free list
}

// Set memory to a specific value
void* memset(void* dest, int val, size_t count) {
    uint8_t* d = (uint8_t*)dest;

    for (size_t i = 0; i < count; i++) {
        d[i] = (uint8_t)val;
    }
    return dest;
}

// Copy memory from source to destination
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Reallocate memory
void* realloc(void* ptr, size_t new_size) {
    if (ptr == NULL) {
        return malloc(new_size); // If ptr is NULL, behave like malloc
    }

    struct mem_block* old_block = (struct mem_block*)((uint8_t*)ptr - sizeof(struct mem_block));
    size_t old_size = old_block->size;

    // If the new size is less than or equal to the old size, return the same pointer
    if (new_size <= old_size) {
        return ptr;
    }

    // Allocate new memory
    void* new_ptr = malloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size); // Copy the old data to the new memory
        free(ptr); // Free the old memory
    }

    return new_ptr;
}

#endif // MEMORY_H