#pragma once

#include <types.h>
#include <debug/Assert.h>

namespace Utils::Bits
{
  inline bool IsAligned(uintptr_t address, size_t alignment)
  {
    return (address & (alignment - 1)) == 0;
  }

  uintptr_t AlignUp(uintptr_t address, size_t alignment);
  uintptr_t AlignDown(uintptr_t address, size_t alignment);
}
