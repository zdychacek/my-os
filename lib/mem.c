#include "mem.h"
#include "../drivers/display.h"
#include "../lib/function.h"

uint32_t last_alloc = 0;
uint32_t heap_begin = 0;
uint32_t heap_end = 0;
uint32_t memory_used = 0;

void memory_init(uint32_t kernel_end)
{
  last_alloc = kernel_end + 0x1000;
  heap_begin = last_alloc;
  heap_end = 0x400000;

  memset((char *)heap_begin, 0, heap_end - heap_begin);

  kprintf("Memory initialized. Kernel heap starts at 0x%x...\n", last_alloc);
}

void memory_print_info()
{
  kprintf("Memory used: %d bytes\n", memory_used);
  kprintf("Memory free: %d bytes\n", heap_end - heap_begin - memory_used);
  kprintf("Heap size: %d bytes\n", heap_end - heap_begin);
  kprintf("Heap start: 0x%x\n", heap_begin);
  kprintf("Heap end: 0x%x\n", heap_end);
}

void *malloc(size_t size)
{
  if (!size)
  {
    return 0;
  }

  // Loop through blocks and find a block sized the same or bigger
  uint8_t *mem = (uint8_t *)heap_begin;

  while ((uint32_t)mem < last_alloc)
  {
    alloc_t *a = (alloc_t *)mem;

    // If the alloc has no size, we have reaced the end of allocation
    if (!a->size)
      goto nalloc;

    /* If the alloc has a status of 1 (allocated), then add its size
		 * and the sizeof alloc_t to the memory and continue looking.
		 */
    if (a->status)
    {
      mem += a->size;
      mem += sizeof(alloc_t);
      //mem += 4;

      continue;
    }
    /* If the is not allocated, and its size is bigger or equal to the
		 * requested size, then adjust its size, set status and return the location.
		 */
    if (a->size >= size)
    {
      // Set to allocated
      a->status = 1;

      kprintf("Reallocated %d bytes from 0x%x to 0x%x.\n", size, mem + sizeof(alloc_t), mem + sizeof(alloc_t) + size);

      memset(mem + sizeof(alloc_t), 0, size);
      memory_used += size + sizeof(alloc_t);

      return (void *)(mem + sizeof(alloc_t));
    }
    /* If it isn't allocated, but the size is not good, then
		 * add its size and the sizeof alloc_t to the pointer and
		 * continue;
		 */
    mem += a->size;
    mem += sizeof(alloc_t);
    //mem += 4;
  }

nalloc:;
  if (last_alloc + size + sizeof(alloc_t) >= heap_end)
  {
    panic("Cannot allocate %d bytes! Out of memory.\n", size);
  }

  alloc_t *alloc = (alloc_t *)last_alloc;
  alloc->status = 1;
  alloc->size = size;

  last_alloc += size;
  last_alloc += sizeof(alloc_t);
  last_alloc += 4;

  kprintf("Allocated %d bytes from 0x%x to 0x%x.\n", size, (uint32_t)alloc + sizeof(alloc_t), last_alloc);

  memory_used += size + 4 + sizeof(alloc_t);

  void *dest = (void *)((uint32_t)alloc + sizeof(alloc_t));

  memset(dest, 0, size);

  return dest;
}

void free(void *mem)
{
  alloc_t *alloc = (mem - sizeof(alloc_t));

  memory_used -= alloc->size + sizeof(alloc_t);
  alloc->status = 0;
}

void memcpy(void *dest, const void *src, size_t n)
{
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  for (size_t i = 0; i < n; i++)
  {
    cdest[i] = csrc[i];
  }
}

void memset(void *dest, int val, size_t n)
{
  uint8_t *temp = (uint8_t *)dest;

  for (; n != 0; n--)
    *temp++ = val;
}
