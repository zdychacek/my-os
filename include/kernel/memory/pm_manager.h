
#pragma once

#include "lib/types.h"

// physical address
typedef uint32_t physical_addr;

// initialize the physical memory manager
extern void pmm_init(size_t, physical_addr);

// enables a physical memory region for use
extern void pmm_init_region(physical_addr, size_t);

// disables a physical memory region as in use (unuseable)
extern void pmm_deinit_region(physical_addr base, size_t);

// allocates a single memory block
extern void *pmm_alloc_block();

// releases a memory block
extern void pmm_free_block(void *);

// allocates blocks of memory
extern void *pmm_alloc_blocks(size_t);

// frees blocks of memory
extern void pmm_free_blocks(void *, size_t);

// returns amount of physical memory the manager is set to use
extern size_t pmm_get_memory_size();

// returns number of blocks currently in use
extern uint32_t pmm_get_used_block_count();

// returns number of blocks not in use
extern uint32_t pmm_get_free_block_count();

// returns number of memory blocks
extern uint32_t pmm_get_block_count();

// returns default memory block size in bytes
extern uint32_t pmm_get_block_size();

// enable or disable paging
extern void pmm_paging_enable(bool);

// test if paging is enabled
extern bool pmm_is_paging();

// load the Page Directory Base Register (PDBR)  physical address
extern void pmm_load_PDBR(physical_addr);

// get the Page Directory Base Register (PDBR) physical address
extern physical_addr pmm_get_PDBR();
