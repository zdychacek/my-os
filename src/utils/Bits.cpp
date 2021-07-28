#include <utils/Bits.h>
#include <debug/Assert.h>

namespace Utils::Bits
{
  uintptr_t AlignUp(uintptr_t address, size_t alignment)
  {
    Assert(IsAligned(alignment, 2), "Alignment must be of power of two");

    if (address % alignment != 0)
    {
      address += alignment - address % alignment;
    }

    return address;
  }

  uintptr_t AlignDown(uintptr_t address, size_t alignment)
  {
    Assert(IsAligned(alignment, 2), "Alignment must be of power of two");

    if (address % alignment != 0)
    {
      address -= address % alignment;
    }

    return address;
  }
}
