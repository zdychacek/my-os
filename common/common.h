#pragma once

#include <stdint.h>

typedef struct _boot_mmap
{
  uint64_t base;
  uint64_t len;
  uint64_t type;
} mmap;

// Sometimes we want to keep parameters to a function for later use
// and this is a solution to avoid the 'unused parameter' compiler warning */
#define UNUSED(x) (void)(x)
#define NULL ((void *)0)
