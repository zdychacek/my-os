#pragma once

#include <stdint.h>

typedef struct _memory_region
{
  uint64_t base;
  uint64_t len;
  uint64_t type;
} memory_region;

// Sometimes we want to keep parameters to a function for later use
// and this is a solution to avoid the 'unused parameter' compiler warning */
#define UNUSED(x) (void)(x)
#define NULL ((void *)0)
