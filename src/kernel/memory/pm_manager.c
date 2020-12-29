
#include "kernel/memory/pm_manager.h"
#include "kernel/drivers/display.h"
#include "lib/string.h"

// #define PRINT_MEMORY_MAP

// defined in ASM
extern void _load_page_directory(physical_addr addr);
extern physical_addr _get_page_directory();
extern bool _enable_paging();
extern bool _is_paging_enabled();

#ifdef PRINT_MEMORY_MAP
static char *str_memory_types[] = {
    "Available",
    "Reserved",
    "ACPI Reclaim",
    "ACPI NVS Memory"};
#endif

// 8 blocks per byte
#define PMMNGR_BLOCKS_PER_BYTE 8

// block size (4k)
#define PMMNGR_BLOCK_SIZE 4096

// block alignment
#define PMMNGR_BLOCK_ALIGN PMMNGR_BLOCK_SIZE

// size of physical memory
static uint32_t _pmm_memory_size = 0;

// number of blocks currently in use
static uint32_t _pmm_used_blocks = 0;

// maximum number of available memory blocks
static uint32_t _pmm_max_blocks = 0;

// memory map bit array. Each bit represents a memory block
static uint32_t *_pmm_memory_map = 0;

// memory map bit array size
static uint32_t _pmm_memory_map_size = 0;

// finds first free frame in the bit array and returns its index
int mmap_first_free();

// finds first free "size" number of frames and returns its index
int mmap_first_free_s(size_t size);

// set any bit (frame) within the memory map bit array
void mmap_set(int bit)
{
  _pmm_memory_map[bit / 32] |= (1 << (bit % 32));
}

// unset any bit (frame) within the memory map bit array
void mmap_unset(int bit)
{
  _pmm_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

// test if any bit (frame) is set within the memory map bit array
bool mmap_test(int bit)
{
  return _pmm_memory_map[bit / 32] & (1 << (bit % 32));
}

// finds first free frame in the bit array and returns its index
int mmap_first_free()
{
  // find the first free bit
  for (uint32_t i = 0; i < pmm_get_block_count(); i++)
    if (_pmm_memory_map[i] != 0xffffffff)
    {
      // test each bit in the dword
      for (int j = 0; j < 32; j++)
      {
        int bit = 1 << j;

        if (!(_pmm_memory_map[i] & bit))
        {
          return i * 32 + j;
        }
      }
    }

  return -1;
}

// finds first free "size" number of frames and returns its index
int mmap_first_free_s(size_t size)
{
  if (size == 0)
  {
    return -1;
  }

  if (size == 1)
  {
    return mmap_first_free();
  }

  for (uint32_t i = 0; i < pmm_get_block_count(); i++)
  {
    if (_pmm_memory_map[i] != 0xffffffff)
    {
      // test each bit in the dword
      for (int j = 0; j < 32; j++)
      {
        int bit = 1 << j;

        if (!(_pmm_memory_map[i] & bit))
        {
          int starting_bit = i * 32;
          starting_bit += bit; // get the free bit in the dword at index i

          uint32_t free = 0; // loop through each bit to see if its enough space

          for (uint32_t count = 0; count <= size; count++)
          {
            if (!mmap_test(starting_bit + count))
            {
              free++; // this bit is clear (free frame)
            }

            if (free == size)
            {
              return i * 32 + j; // free count==size needed; return index
            }
          }
        }
      }
    }
  }

  return -1;
}

void pmm_init(size_t memory_size, physical_addr kernel_start, physical_addr kernel_end, memory_region *mmap_addr, size_t mmap_len)
{
  _pmm_memory_size = memory_size;
  _pmm_memory_map = (uint32_t *)kernel_end;
  _pmm_max_blocks = (pmm_get_memory_size() * 1024) / PMMNGR_BLOCK_SIZE;
  _pmm_used_blocks = pmm_get_block_count();
  _pmm_memory_map_size = pmm_get_block_count() / PMMNGR_BLOCKS_PER_BYTE;

  // By default, all of memory is in use
  memset(_pmm_memory_map, 0xffffffff, pmm_get_block_count() / PMMNGR_BLOCKS_PER_BYTE);

  memory_region *region = mmap_addr;
  size_t regions_count = mmap_len / sizeof(memory_region);

#ifdef PRINT_MEMORY_MAP
  kprint("(PMM) Memory map:\n");
#endif

  for (size_t i = 0; i < regions_count; i++)
  {
    char type = (char)region[i].type;

    if (type == MEMORY_REGION_AVAILABLE)
    {
      pmm_init_region(region[i].base, region[i].len);
    }
#ifdef PRINT_MEMORY_MAP
    kprintf("0x%x-0x%x %s %d KB\n",
            (uint32_t)region[i].base,
            (uint32_t)(region[i].base + region[i].len),
            str_memory_types[region[i].type - 1],
            region[i].len / 1024);
#endif
  }

  // exclude kernel
  pmm_deinit_region(kernel_start, kernel_end - kernel_start);

  // exclude memory map bitmap
  pmm_deinit_region((physical_addr)_pmm_memory_map, _pmm_memory_map_size);

  kprintf("(PMM) Initialized with %d KB\n", memory_size);
  kprintf("(PMM) Allocation blocks count: %d (block size: %d bytes)\n",
          pmm_get_block_count(), pmm_get_block_size());
  kprintf("(PMM) Free allocation blocks count: %d\n", pmm_get_free_block_count());
}

void pmm_init_region(physical_addr base, size_t size)
{
  int align = base / PMMNGR_BLOCK_SIZE;
  int blocks = size / PMMNGR_BLOCK_SIZE;

  for (; blocks > 0; blocks--)
  {
    mmap_unset(align++);
    _pmm_used_blocks--;
  }

  mmap_set(0); // first block is always set. This insures allocs cant be 0
}

void pmm_deinit_region(physical_addr base, size_t size)
{
  int align = base / PMMNGR_BLOCK_SIZE;
  int blocks = size / PMMNGR_BLOCK_SIZE;

  for (; blocks > 0; blocks--)
  {
    mmap_set(align++);
    _pmm_used_blocks++;
  }

  mmap_set(0); // first block is always set. This insures allocs cant be 0
}

void *pmm_alloc_block()
{
  if (pmm_get_free_block_count() <= 0)
  {
    return NULL; // out of memory
  }

  int frame = mmap_first_free();

  if (frame == -1)
  {
    return NULL; // out of memory
  }

  mmap_set(frame);

  physical_addr address = frame * PMMNGR_BLOCK_SIZE;
  _pmm_used_blocks++;

  return (void *)address;
}

void pmm_free_block(void *block)
{
  physical_addr address = (physical_addr)block;
  int frame = address / PMMNGR_BLOCK_SIZE;

  mmap_unset(frame);

  _pmm_used_blocks--;
}

void *pmm_alloc_blocks(size_t size)
{
  if (pmm_get_free_block_count() <= size)
  {
    return NULL; // not enough space
  }

  int frame = mmap_first_free_s(size);

  if (frame == -1)
  {
    return NULL; // not enough space
  }

  for (uint32_t i = 0; i < size; i++)
  {
    mmap_set(frame + i);
  }

  physical_addr address = frame * PMMNGR_BLOCK_SIZE;

  _pmm_used_blocks += size;

  return (void *)address;
}

void pmm_free_blocks(void *block, size_t size)
{
  physical_addr address = (physical_addr)block;
  int frame = address / PMMNGR_BLOCK_SIZE;

  for (uint32_t i = 0; i < size; i++)
  {
    mmap_unset(frame + i);
  }

  _pmm_used_blocks -= size;
}

size_t pmm_get_memory_size()
{
  return _pmm_memory_size;
}

uint32_t pmm_get_block_count()
{
  return _pmm_max_blocks;
}

uint32_t pmm_get_used_block_count()
{
  return _pmm_used_blocks;
}

uint32_t pmm_get_free_block_count()
{
  return _pmm_max_blocks - _pmm_used_blocks;
}

uint32_t pmm_get_block_size()
{
  return PMMNGR_BLOCK_SIZE;
}

void pmm_enable_paging(bool set)
{
  _enable_paging(set);
}

bool pmm_is_paging_enabled()
{
  return _is_paging_enabled();
}

void pmm_load_PDBR(physical_addr addr)
{
  _load_page_directory(addr);
}

physical_addr pmm_get_PDBR()
{
  return _get_page_directory();
}
