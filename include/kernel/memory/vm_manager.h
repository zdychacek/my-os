#pragma once

#include "lib/types.h"
#include "kernel/memory/vm_pte.h"
#include "kernel/memory/vm_pde.h"

// virtual address
typedef uint32_t virtual_addr;

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22))
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

// page table
typedef struct _ptable
{
  pt_entry entries[PAGES_PER_TABLE];
} ptable;

// page directory
typedef struct _pdirectory
{
  pd_entry entries[PAGES_PER_DIR];
} pdirectory;

// initialize the memory manager
extern void vmm_init();

// maps phys to virtual address
extern void vmm_map_page(void *phys, void *virt);

// allocates a page in physical memory
extern bool vmm_alloc_page(pt_entry *entry);

// frees a page in physical memory
extern void vmm_free_page(pt_entry *entry);

// switch to a new page directory
extern bool vmm_switch_directory(pdirectory *directory);

// get current page directory
extern pdirectory *vmm_get_directory();

// flushes a cached translation lookaside buffer (TLB) entry
extern void vmm_flush_tlb_entry(virtual_addr address);

// clears a page table
extern void vmm_ptable_clear(ptable *table);

// convert virtual address to page table index
extern uint32_t vmm_ptable_virt_to_index(virtual_addr address);

// get page entry from page table
extern pt_entry *vmm_ptable_lookup_entry(ptable *table, virtual_addr address);

// convert virtual address to page directory index
extern uint32_t vmm_pdirectory_virt_to_index(virtual_addr address);

// clears a page directory table
extern void vmm_pdirectory_clear(pdirectory *directory);

// get directory entry from directory table
extern pd_entry *vmm_pdirectory_lookup_entry(pdirectory *directory, virtual_addr address);
