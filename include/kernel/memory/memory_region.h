#pragma once

#include "lib/types.h"

typedef struct _memory_region
{
  uint64_t base;
  uint64_t len;
  uint64_t type;
} memory_region;

enum MEMORY_REGION_TYPE
{
  MEMORY_REGION_AVAILABLE = 1,
  MEMORY_REGION_RESERVED,
  MEMORY_REGION_ACPI_RECLAIM,
  MEMORY_REGION_ACPI_NVS,
};
