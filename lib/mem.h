#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
  uint8_t status;
  uint32_t size;
} alloc_t;

void memory_init(uint32_t kernel_end);
void memory_print_info();

void *malloc(size_t size);
void free(void *mem);
void memcpy(void *dest, const void *src, size_t len);
void memset(void *dest, int val, size_t n);
