#pragma once

#include <types.h>

static constexpr uint64_t PhysicalMemoryOffset = 0xffffc00000000000;

class PhysicalMemoryManager
{
private:
  PhysicalMemoryManager() {}

public:
  template <typename T>
  T *ConvertPhysicalAddressToVirtual(T *ptr)
  {
    uintptr_t virtualAddress = (uintptr_t)ptr;

    virtualAddress += PhysicalMemoryOffset;

    return (T *)virtualAddress;
  }

  inline uintptr_t ConvertPhysicalAddressToVirtual(uintptr_t address)
  {
    return address + PhysicalMemoryOffset;
  }

  static PhysicalMemoryManager *GetInstance()
  {
    static PhysicalMemoryManager instance;

    return &instance;
  }
  PhysicalMemoryManager(PhysicalMemoryManager const &) = delete;
  void operator=(PhysicalMemoryManager const &) = delete;
};
