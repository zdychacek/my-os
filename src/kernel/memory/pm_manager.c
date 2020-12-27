
#include "kernel/memory/pm_manager.h"
#include "lib/string.h"

// 8 blocks per byte
#define PMMNGR_BLOCKS_PER_BYTE 8

// block size (4k)
#define PMMNGR_BLOCK_SIZE 4096

// block alignment
#define PMMNGR_BLOCK_ALIGN PMMNGR_BLOCK_SIZE

// size of physical memory
static uint32_t _mmngr_memory_size = 0;

// number of blocks currently in use
static uint32_t _mmngr_used_blocks = 0;

// maximum number of available memory blocks
static uint32_t _mmngr_max_blocks = 0;

// memory map bit array. Each bit represents a memory block
static uint32_t *_mmngr_memory_map = 0;

// finds first free frame in the bit array and returns its index
int mmap_first_free();

// finds first free "size" number of frames and returns its index
int mmap_first_free_s(size_t size);

// set any bit (frame) within the memory map bit array
void mmap_set(int bit)
{
  _mmngr_memory_map[bit / 32] |= (1 << (bit % 32));
}

// unset any bit (frame) within the memory map bit array
void mmap_unset(int bit)
{
  _mmngr_memory_map[bit / 32] &= ~(1 << (bit % 32));
}

// test if any bit (frame) is set within the memory map bit array
bool mmap_test(int bit)
{
  return _mmngr_memory_map[bit / 32] & (1 << (bit % 32));
}

// finds first free frame in the bit array and returns its index
int mmap_first_free()
{
  // find the first free bit
  for (uint32_t i = 0; i < pmm_get_block_count(); i++)
    if (_mmngr_memory_map[i] != 0xffffffff)
    {
      // test each bit in the dword
      for (int j = 0; j < 32; j++)
      {
        int bit = 1 << j;

        if (!(_mmngr_memory_map[i] & bit))
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
    if (_mmngr_memory_map[i] != 0xffffffff)
    {
      // test each bit in the dword
      for (int j = 0; j < 32; j++)
      {
        int bit = 1 << j;

        if (!(_mmngr_memory_map[i] & bit))
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

void pmm_init(size_t memory_size, physical_addr bitmap)
{
  _mmngr_memory_size = memory_size;
  _mmngr_memory_map = (uint32_t *)bitmap;
  _mmngr_max_blocks = (pmm_get_memory_size() * 1024) / PMMNGR_BLOCK_SIZE;
  _mmngr_used_blocks = pmm_get_block_count();

  // By default, all of memory is in use
  memset(_mmngr_memory_map, 0xf, pmm_get_block_count() / PMMNGR_BLOCKS_PER_BYTE);
}

void pmm_init_region(physical_addr base, size_t size)
{
  int align = base / PMMNGR_BLOCK_SIZE;
  int blocks = size / PMMNGR_BLOCK_SIZE;

  for (; blocks > 0; blocks--)
  {
    mmap_unset(align++);
    _mmngr_used_blocks--;
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
    _mmngr_used_blocks++;
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

  physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
  _mmngr_used_blocks++;

  return (void *)addr;
}

void pmm_free_block(void *p)
{
  physical_addr addr = (physical_addr)p;
  int frame = addr / PMMNGR_BLOCK_SIZE;

  mmap_unset(frame);

  _mmngr_used_blocks--;
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

  physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
  _mmngr_used_blocks += size;

  return (void *)addr;
}

void pmm_free_blocks(void *p, size_t size)
{
  physical_addr addr = (physical_addr)p;
  int frame = addr / PMMNGR_BLOCK_SIZE;

  for (uint32_t i = 0; i < size; i++)
  {
    mmap_unset(frame + i);
  }

  _mmngr_used_blocks -= size;
}

size_t pmm_get_memory_size()
{
  return _mmngr_memory_size;
}

uint32_t pmm_get_block_count()
{
  return _mmngr_max_blocks;
}

uint32_t pmm_get_used_block_count()
{
  return _mmngr_used_blocks;
}

uint32_t pmm_get_free_block_count()
{
  return _mmngr_max_blocks - _mmngr_used_blocks;
}

uint32_t pmm_get_block_size()
{
  return PMMNGR_BLOCK_SIZE;
}

void pmm_paging_enable(bool b)
{
  asm volatile("  mov %%cr0, %%eax\n"
               "  cmp	$1, %0\n"
               "  je enable\n"
               "  jmp	disable\n"
               "enable:\n"
               "  or $0x80000000, %%eax\n"
               "  mov %%eax, %%cr0\n"
               "  jmp	done\n"
               "disable:\n"
               "  and $0x7FFFFFFF, %%eax\n"
               "  mov %%eax, %%cr0\n"
               "done:"
               :
               : "c"(b));
}

bool pmm_is_paging()
{
  uint32_t cr0 = 0;

  asm volatile("mov %%cr0, %%eax\n"
               "mov %%eax, %0"
               : "=r"(cr0)
               :);

  return (cr0 & 0x80000000) ? false : true;
}

void pmm_load_PDBR(physical_addr addr)
{
  asm volatile("mov %0, %%eax\n"
               "mov %%eax, %%cr3"
               :
               : "r"(addr));
}

physical_addr pmm_get_PDBR()
{
  physical_addr pdbr = 0;

  asm volatile("mov %%cr3, %%eax\n"
               "mov %%eax, %0"
               : "=r"(pdbr)
               :);

  return pdbr;
}
