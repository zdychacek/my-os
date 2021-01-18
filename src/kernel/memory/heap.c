#include "kernel/memory/heap.h"
#include "kernel/memory/pm_manager.h"
#include "kernel/memory/vm_manager.h"
#include "kernel/drivers/display.h"
#include "kernel/panic.h"
#include "lib/string.h"

#define HEAP_MAGIC 0xdeadbeef

typedef struct _block_header
{
  uint32_t magic;
  bool used;
  uint32_t size;
  struct _block_header *next;
} block_header;

static block_header *heap_top = (block_header *)HEAP_START;
static block_header block_head = {HEAP_MAGIC, true, 0, NULL};

// Create a free block of at least `size` bytes. It allocates the nearest greater multiply of 4096 bytes.

// TODO: make it accept negative parameter to lower program break by unmapping unneeded pages
// TODO: validate heap upper bound 0xffbfffff
static block_header *more_heap(size_t size)
{
  block_header *new_block = heap_top;

  // Number of pages needed to allocate.
  size_t pages_count =
      (size + sizeof(block_header) % PAGE_SIZE == 0)
          ? (size + sizeof(block_header)) / PAGE_SIZE
          : (size + sizeof(block_header)) / PAGE_SIZE + 1;

  size_t allocated_pages_count;

  for (allocated_pages_count = 0; allocated_pages_count < pages_count; allocated_pages_count++)
  {
    void *frame = pmm_alloc_frame();

    if (!frame)
    {
      kprint("Can't allocate physical frame for kernel heap.\n");

      break;
    }

    vmm_map_page((physical_addr)frame, (virtual_addr)heap_top);

    heap_top = (block_header *)((char *)heap_top + PAGE_SIZE);
  }

  new_block->size = PAGE_SIZE * allocated_pages_count - sizeof(block_header);
  new_block->used = false;
  new_block->magic = HEAP_MAGIC;
  new_block->next = NULL;

  return new_block;
}

// Makes one free block from more free contiguous blocks.
static inline void merge_free_blocks(block_header *block)
{
  while (!block->used && block->next && !block->next->used)
  {
    if (block->magic != HEAP_MAGIC)
    {
      kernel_panic("Heap corrupted");

      return;
    }

    block->size += block->next->size + sizeof(block_header);
    block->next = block->next->next;
  }
}

// Make two smaller blocks from larger one. One of the new blocks is marked as used and one is free.
static block_header *cut_block(block_header *block, size_t size)
{
  // A block with a required size fits in the `block` block. There is also space for `malloc_block` and a new free block.
  if (block->size > size + sizeof(block_header))
  {
    block_header *new_block = (block_header *)((char *)block + size + sizeof(block_header));

    new_block->size = block->size - size - sizeof(block_header);
    new_block->used = false;
    new_block->magic = HEAP_MAGIC;
    new_block->next = block->next;

    // A new free block is created. There might be a free block next to the it.
    merge_free_blocks(new_block);

    block->size = size;
    block->used = true;
    block->next = new_block;

    return (block_header *)((char *)block + sizeof(block_header));
  }
  // There is a space for the required size, but the `malloc_block` does not fit in, so a new free block can not be created.
  else if (block->size >= size && block->size <= size + sizeof(block_header))
  {
    block->used = true;

    return (block_header *)((char *)block + sizeof(block_header));
  }
  else
  {
    kernel_panic("Reached unreachable code.");

    return NULL;
  }
}

void heap_dump()
{
  for (block_header *block = block_head.next; block; block = block->next)
  {
    if (block->magic != HEAP_MAGIC)
    {
      kernel_panic("Heap corrupted");

      return;
    }

    kprintf("address: 0x%x, size: %d, state: %s, next: %x\n",
            block, block->size, block->used ? "used" : "free", block->next);
  }
}

void *malloc(size_t size)
{
  if (size == 0)
  {
    return NULL;
  }

  block_header *block;
  void *retval;

  for (block = &block_head; block->next; block = block->next)
  {
    if (block->magic != HEAP_MAGIC)
    {
      kernel_panic("Heap corrupted");

      return NULL;
    }

    // `block` is not used and a block of required size fits in.
    if (!(block->used || block->size < size))
    {
      retval = cut_block(block, size);
      goto ret;
    }
  }

  if (block->magic != HEAP_MAGIC)
  {
    kernel_panic("Heap corrupted");

    retval = NULL;
    goto ret;
  }

  // This is for the last block.
  if (!(block->used || block->size < size))
  {
    retval = cut_block(block, size);
    goto ret;
  }

  if (block->used)
  {
    block->next = more_heap(size);

    if (block->next->size < size)
    {
      // `more_heap()` may allocate less memory than required when the system is out of memory.
      retval = NULL;
    }
    else
    {
      // `more_heap()` may allocate more memory than required.
      retval = cut_block(block->next, size);
    }
  }
  else
  {
    // The block at the end is free, so it can be merged with the newly created block. We can allocate less memory.
    block->next = more_heap(size - block->size - sizeof(block_header));

    merge_free_blocks(block);

    // `blk->next` is NULL, because we merged the last blocks.
    if (block->size < size)
    {
      retval = NULL;
    }
    else
    {
      retval = cut_block(block, size);
    }
  }

ret:
  return retval;
}

void free(void *addr)
{
  // `addr` is not the address of a block, but the address of the first usable byte after `malloc_block`.
  block_header *block_to_be_freed = addr - sizeof(block_header);

  for (block_header *block = block_head.next; block; block = block->next)
  {
    if (block->magic != HEAP_MAGIC)
    {
      kernel_panic("Heap corrupted");

      return;
    }

    if (block == block_to_be_freed)
    {
      if (block->used == false)
      {
        kprint("free(): Trying to free a block twice");

        return;
      }

      block->used = false;

      // We need to merge the free blocks before the newly created block as well as after.
      // `merge_free_blocks()` merges only after, because you can not move backwards in the block list.
      for (block = block_head.next; block; block = block->next)
      {
        merge_free_blocks(block);
      }

      return;
    }
  }

  kprint("free(): Trying to free a block which is not allocated.\n");
}

void *realloc(void *addr, size_t size)
{
  block_header *block_to_be_freed = addr - sizeof(block_header);

  for (block_header *block = block_head.next; block; block = block->next)
  {
    if (block->magic != HEAP_MAGIC)
    {
      kernel_panic("Heap corrupted");

      return NULL;
    }

    if (block == block_to_be_freed)
    {
      if (block->used == false)
      {
        kprint("realloc(): Trying to reallocate a block which is not allocated.\n");

        return NULL;
      }

      if (size == 0)
      {
        free(addr);

        return NULL;
      }
      else if (block->size == size)
      {
        return block;
      }
      else if (block->size > size)
      {
        // shrinking
        return cut_block(block, size);
      }
      else
      {
        // enlarging
        // `block` might be the last block. In that case we should free it at first and its memory can be reused in the forthcoming `malloc()`.
        free(addr);

        void *new_addr = malloc(size);

        // The new block may be same as the last block, but enlarged.
        if (addr != new_addr)
        {
          memcpy(new_addr, addr, block->size);
        }

        return new_addr;
      }
    }
  }

  kprint("realloc(): Trying to reallocate a block which is not allocated.\n");

  return NULL;
}

void heap_init()
{
  kprint("Kernel heap initialized\n");
}
