#pragma once

#include "lib/types.h"

#define HEAP_START 0xc0400000
#define HEAP_END 0xffbfffff

void *malloc(size_t size);
void free(void *addr);
void *realloc(void *addr, size_t size);

void heap_init();
void heap_dump();
