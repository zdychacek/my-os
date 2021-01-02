
#pragma once

#include "lib/types.h"
#include "kernel/memory/memory_region.h"

// physical address
typedef uint32_t physical_addr;

// initialize the physical memory manager
extern void pmm_init(size_t memory_size_kb, physical_addr kernel_start, physical_addr kernel_end, memory_region *mmap_addr, size_t mmap_len);

// enables a physical memory region for use
extern void pmm_init_region(physical_addr base, size_t size);

// disables a physical memory region as in use (unusable)
extern void pmm_deinit_region(physical_addr base, size_t size);

// allocates a single memory frame
extern void *pmm_alloc_frame();

// allocates frames of memory
extern void *pmm_alloc_frames(size_t);

// releases a memory frame
extern void pmm_free_frame(void *);

// frees frames of memory
extern void pmm_free_frames(void *, size_t);

// returns amount of physical memory the manager is set to use
extern size_t pmm_get_memory_size();

// returns number of frames currently in use
extern uint32_t pmm_get_used_frames_count();

// returns number of frames not in use
extern uint32_t pmm_get_free_frames_count();

// returns number of memory frames
extern uint32_t pmm_get_frames_count();

// returns default memory frame size in bytes
extern uint32_t pmm_get_frame_size();

// enable or disable paging
extern void pmm_enable_paging(bool);

// test if paging is enabled
extern bool pmm_is_paging_enabled();

// load the Page Directory Base Register (PDBR)  physical address
extern void pmm_load_PDBR(physical_addr);

// get the Page Directory Base Register (PDBR) physical address
extern physical_addr pmm_get_PDBR();
