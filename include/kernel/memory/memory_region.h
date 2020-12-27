#pragma once

#include "lib/types.h"

typedef struct _memory_region
{
  uint64_t base;
  uint64_t len;
  uint64_t type;
} memory_region;
