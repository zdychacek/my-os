#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct _alloc_t
{
    uint8_t status;
    uint32_t size;
} alloc_t;

// TODO(ondrej): move into physical memory manager
void memory_init(uint32_t kernel_end);
void memory_print_info();

void *malloc(size_t size);
void free(void *mem);