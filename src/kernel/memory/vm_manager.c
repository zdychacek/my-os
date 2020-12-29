#include "kernel/memory/vm_manager.h"
#include "kernel/memory/pm_manager.h"
#include "lib/string.h"

// defined in ASM
extern void _flush_tlb_entry(virtual_addr address);

// page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// page sizes are 4k
#define PAGE_SIZE 4096

// current directory table
pdirectory *_current_directory = 0;

// current page directory base register
physical_addr _current_pdbr = 0;

inline pt_entry *vmm_ptable_lookup_entry(ptable *page, virtual_addr address)
{
  if (page)
  {
    return &page->entries[PAGE_TABLE_INDEX(address)];
  }

  return 0;
}

inline pd_entry *vmm_pdirectory_lookup_entry(pdirectory *page, virtual_addr address)
{
  if (page)
  {
    return &page->entries[PAGE_TABLE_INDEX(address)];
  }

  return 0;
}

inline bool vmm_switch_directory(pdirectory *directory)
{
  if (!directory)
  {
    return false;
  }

  _current_directory = directory;

  pmm_load_PDBR(_current_pdbr);

  return true;
}

void vmm_flush_tlb_entry(virtual_addr address)
{
  _flush_tlb_entry(address);
}

pdirectory *vmm_get_directory()
{
  return _current_directory;
}

bool vmm_alloc_page(pt_entry *entry)
{
  // allocate a free physical frame
  void *page = pmm_alloc_block();

  if (!page)
  {
    return false;
  }

  // map it to the page
  pt_entry_set_frame(entry, (physical_addr)page);
  pt_entry_add_attribute(entry, I86_PTE_PRESENT);

  return true;
}

void vmm_free_page(pt_entry *entry)
{
  void *page = (void *)pt_entry_get_frame(*entry);

  if (page)
  {
    pmm_free_block(page);
  }

  pt_entry_remove_attribute(entry, I86_PTE_PRESENT);
}

void vmm_map_page(void *phys, void *virt)
{
  // get page directory
  pdirectory *directory = vmm_get_directory();

  // get page table
  pd_entry *directory_entry = &directory->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

  if ((*directory_entry & I86_PTE_PRESENT) != I86_PTE_PRESENT)
  {
    // page table not present, allocate it
    ptable *table = (ptable *)pmm_alloc_block();

    if (!table)
    {
      return;
    }

    // clear page table
    memset(table, 0, sizeof(ptable));

    // create a new entry
    pd_entry *entry = &directory->entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

    // map in the table
    pd_entry_add_attribute(entry, I86_PDE_PRESENT);
    pd_entry_add_attribute(entry, I86_PDE_WRITABLE);
    pd_entry_set_frame(entry, (physical_addr)table);
  }

  // get table
  ptable *table = (ptable *)PAGE_GET_PHYSICAL_ADDRESS(directory_entry);

  // get page
  pt_entry *page = &table->entries[PAGE_TABLE_INDEX((uint32_t)virt)];

  // map it in
  pt_entry_set_frame(page, (physical_addr)phys);
  pt_entry_add_attribute(page, I86_PTE_PRESENT);
}

void vmm_init()
{
  ptable *table = (ptable *)pmm_alloc_block();

  if (!table)
  {
    return;
  }

  // clear page table
  memset(table, 0, sizeof(ptable));

  // 1st 4mb are idenitity mapped
  for (int i = 0, frame = 0x0, virt = 0x00000000; i < 1024; i++, frame += 4096, virt += 4096)
  {
    // create a new page
    pt_entry page = 0;

    pt_entry_add_attribute(&page, I86_PTE_PRESENT);
    pt_entry_set_frame(&page, frame);

    // ...and add it to the page table
    table->entries[PAGE_TABLE_INDEX(virt)] = page;
  }

  // create default directory table
  pdirectory *directory = (pdirectory *)pmm_alloc_blocks(2);

  if (!directory)
  {
    return;
  }

  // clear directory table and set it as current
  memset(directory, 0, sizeof(pdirectory));

  pd_entry *entry = &directory->entries[PAGE_DIRECTORY_INDEX(0x00000000)];
  pd_entry_add_attribute(entry, I86_PDE_PRESENT);
  pd_entry_add_attribute(entry, I86_PDE_WRITABLE);
  pd_entry_set_frame(entry, (physical_addr)table);

  // store current PDBR
  _current_pdbr = (physical_addr)&directory->entries;

  // switch to our page directory
  vmm_switch_directory(directory);

  // enable paging
  pmm_enable_paging(true);
}
