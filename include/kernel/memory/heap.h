#pragma once

#include "lib/types.h"

void *malloc(size_t size);
void free(void *addr);
void *realloc(void *addr, size_t size);

void heap_init(uint32_t heap_start, uint32_t heap_max);
void heap_dump();
size_t heap_get_used_space();
size_t heap_get_free_space();
