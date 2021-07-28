#include <types.h>
#include <memory/PhysicalMemoryManager.h>

static uintptr_t heapBase = 0x2000000;

void *__attribute__((weak)) operator new(long unsigned int size)
{
  // TODO: replace with heap allocator when done
  void *pointer = (void *)(PhysicalMemoryManager::GetInstance()->ConvertPhysicalAddressToVirtual(heapBase));

  heapBase += size;

  return pointer;
}

void *__attribute__((weak)) operator new[](long unsigned int size)
{
  // TODO: replace with heap allocator when done
  void *pointer = (void *)(PhysicalMemoryManager::GetInstance()->ConvertPhysicalAddressToVirtual(heapBase));

  heapBase += size;

  return pointer;
}

void __attribute__((weak)) operator delete(void *ptr)
{
  unused(ptr);
}

void __attribute__((weak)) operator delete[](void *ptr)
{
  unused(ptr);
}

void __attribute__((weak)) operator delete(void *ptr, long unsigned int size)
{
  unused(size);
  unused(ptr);
}

void __attribute__((weak)) operator delete[](void *ptr, long unsigned int size)
{
  unused(size);
  unused(ptr);
}
