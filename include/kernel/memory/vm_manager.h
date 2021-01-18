#pragma once

#include "lib/types.h"
#include "kernel/memory/vm_pte.h"
#include "kernel/memory/vm_pde.h"

// virtual address
typedef uint32_t virtual_addr;

// Err code interpretation
#define PAGING_ERR_PRESENT 0x1
#define PAGING_ERR_RW 0x2
#define PAGING_ERR_USER 0x4

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR 1024

// page sizes are 4k
#define PAGE_SIZE 4096

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

// helper indexer structure
typedef struct _page_index
{
  uint32_t page_directory;
  uint32_t page_table;
} __attribute__((packed)) page_index;

// initialize the memory manager
extern void vmm_init();

// maps physical address into virtual memory
void vmm_map_page(physical_addr phys, virtual_addr virt);

// unmaps page from virtual memory
void vmm_unmap_page(virtual_addr virt);

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

// get page entry from page table
extern pt_entry *vmm_ptable_lookup_entry(ptable *table, virtual_addr address);

// clears a page directory table
extern void vmm_pdirectory_clear(pdirectory *directory);

// get directory entry from directory table
extern pd_entry *vmm_pdirectory_lookup_entry(pdirectory *directory, virtual_addr address);

// convert virtual address to page directory and page table index
extern page_index vmm_virt_to_index(virtual_addr address);

// get physical address from page
extern physical_addr vmm_get_phys_addr(uint32_t addr);
