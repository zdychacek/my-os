#include "kernel/memory/vm_manager.h"
#include "kernel/memory/pm_manager.h"
#include "kernel/memory/defs.h"
#include "kernel/hal/idt.h"
#include "kernel/drivers/display.h"
#include "kernel/panic.h"
#include "lib/string.h"

// defined in ASM
extern void _flush_tlb_entry(virtual_addr address);

// page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

// directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

// current directory table
pdirectory *_current_directory = NULL;

// current page directory base register
physical_addr _current_pdbr = 0;

static void page_fault_handler(ir_params *params);
static ptable *vmm_create_page_table(physical_addr phys_from, virtual_addr virt_from);

inline pt_entry *vmm_ptable_lookup_entry(ptable *page, virtual_addr address)
{
  if (page)
  {
    return &page->entries[vmm_virt_to_index(address).page_table];
  }

  return NULL;
}

inline pd_entry *vmm_pdirectory_lookup_entry(pdirectory *page, virtual_addr address)
{
  if (page)
  {
    return &page->entries[vmm_virt_to_index(address).page_directory];
  }

  return NULL;
}

inline bool vmm_switch_directory(pdirectory *directory)
{
  if (!directory)
  {
    return false;
  }

  // store current PDBR
  _current_pdbr = (physical_addr)&directory->entries;

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
  void *frame = pmm_alloc_frame();

  if (!frame)
  {
    return false;
  }

  // map it to the page
  pt_entry_add_attribute(entry, PTE_PRESENT);
  pt_entry_set_frame(entry, (physical_addr)frame);

  return true;
}

void vmm_free_page(pt_entry *entry)
{
  void *frame = (void *)pt_entry_get_frame(*entry);

  if (frame)
  {
    pmm_free_frame(frame);
  }

  pt_entry_remove_attribute(entry, PTE_PRESENT);
}

void vmm_map_page(physical_addr phys, virtual_addr virt)
{
  pdirectory *directory = (pdirectory *)PAGE_DIR_TABLE_VIRT;
  page_index index = vmm_virt_to_index(virt);

  if (directory->entries[index.page_directory] & PDE_PRESENT)
  {
    // page table exists
    ptable *page_table = (ptable *)(PAGE_DIR_VIRT + (index.page_directory * 0x1000)); // virt addr of page table

    if (!page_table->entries[index.page_table] & PTE_PRESENT)
    {
      // page isn't mapped
      pt_entry_add_attribute(&page_table->entries[index.page_table], PTE_PRESENT);
      pt_entry_add_attribute(&page_table->entries[index.page_table], PTE_WRITABLE);
      pt_entry_set_frame(&page_table->entries[index.page_table], phys);
    }
  }
  else
  {
    // doesn't exist, so alloc a page and add into pdir
    ptable *new_page_table = (ptable *)pmm_alloc_frame();
    ptable *page_table = (ptable *)(PAGE_DIR_VIRT + (index.page_directory * 0x1000)); // virt addr of page table

    pd_entry_add_attribute(&directory->entries[index.page_directory], PDE_PRESENT);
    pd_entry_add_attribute(&directory->entries[index.page_directory], PDE_WRITABLE);
    pd_entry_set_frame(&directory->entries[index.page_directory], (physical_addr)new_page_table);

    pt_entry_add_attribute(&page_table->entries[index.page_table], PTE_PRESENT);
    pt_entry_add_attribute(&page_table->entries[index.page_table], PTE_WRITABLE);
    pt_entry_set_frame(&page_table->entries[index.page_table], phys);
  }
}

void vmm_unmap_page(virtual_addr virt)
{
  vmm_flush_tlb_entry(virt);

  pdirectory *directory = (pdirectory *)PAGE_DIR_TABLE_VIRT;
  page_index index = vmm_virt_to_index(virt); // get the PDE and PTE indexes for the addr

  if (directory->entries[index.page_directory] & PDE_PRESENT)
  {
    ptable *page_table = (ptable *)(PAGE_DIR_VIRT + (index.page_directory * 0x1000));

    if (page_table->entries[index.page_table] & PTE_PRESENT)
    {
      // page is mapped, so unmap it
      vmm_free_page(&page_table->entries[index.page_table]);
    }

    int i;

    // check if there are any more present PTEs in this page table
    for (i = 0; i < PAGES_PER_TABLE; i++)
    {
      if (page_table->entries[i] & PTE_PRESENT)
      {
        break;
      }
    }

    // if there are none, then free the space allocated to the page table and delete mappings
    if (i == PAGES_PER_TABLE)
    {
      vmm_free_page(&directory->entries[index.page_directory]);
    }
  }
}

// Creates and fill one page table (4 MB).
static ptable *vmm_create_page_table(physical_addr phys_from, virtual_addr virt_from)
{
  // page table
  ptable *page_table = (ptable *)pmm_alloc_frame();

  if (!page_table)
  {
    return NULL;
  }

  vmm_ptable_clear(page_table);

  for (int i = 0, frame = phys_from, virt = virt_from; i < 1024; i++, frame += 4096, virt += 4096)
  {
    // create a new page
    pt_entry page = 0;

    pt_entry_add_attribute(&page, PTE_PRESENT);
    pt_entry_add_attribute(&page, PTE_WRITABLE);
    pt_entry_set_frame(&page, frame);

    // ...and add it to the page table
    *vmm_ptable_lookup_entry(page_table, (virtual_addr)virt) = page;
  }

  return page_table;
}

void vmm_init()
{
  idt_install_ir_handler(14, page_fault_handler);

  // map first 4mb to 3gb base
  ptable *higher_half_page_table = vmm_create_page_table(0, KERNEL_VIRT_START);

  // allocates first 4MB for framebuffer
  ptable *framebuffer_page_table1 = vmm_create_page_table(FRAMEBUFFER_PHYS_START, FRAMEBUFFER_VIRT_START);

  // allocates second 4MB for framebuffer
  ptable *framebuffer_page_table2 = vmm_create_page_table(FRAMEBUFFER_PHYS_START + 0x400000, FRAMEBUFFER_VIRT_START + 0x400000);

  // create default directory table
  pdirectory *directory = (pdirectory *)pmm_alloc_frame();

  if (!directory)
  {
    return;
  }

  vmm_pdirectory_clear(directory);

  pd_entry *higher_half_entry = vmm_pdirectory_lookup_entry(directory, (virtual_addr)KERNEL_VIRT_START);

  pd_entry_add_attribute(higher_half_entry, PDE_PRESENT);
  pd_entry_add_attribute(higher_half_entry, PDE_WRITABLE);
  pd_entry_set_frame(higher_half_entry, (physical_addr)higher_half_page_table);

  pd_entry *framebuffer_entry1 = vmm_pdirectory_lookup_entry(directory, (virtual_addr)FRAMEBUFFER_VIRT_START);

  pd_entry_add_attribute(framebuffer_entry1, PDE_PRESENT);
  pd_entry_add_attribute(framebuffer_entry1, PDE_WRITABLE);
  pd_entry_set_frame(framebuffer_entry1, (physical_addr)framebuffer_page_table1);

  pd_entry *framebuffer_entry2 = vmm_pdirectory_lookup_entry(directory, (virtual_addr)FRAMEBUFFER_VIRT_START + 0x400000);

  pd_entry_add_attribute(framebuffer_entry2, PDE_PRESENT);
  pd_entry_add_attribute(framebuffer_entry2, PDE_WRITABLE);
  pd_entry_set_frame(framebuffer_entry2, (physical_addr)framebuffer_page_table2);

  /*
    Make last PTE point to page directory itself (recursive page directory setup).

    Allows us to access page tables while in paging mode:

    - virt. address 0xffc00000 points to Page Table #0 (= PTE #0)
    - virt. address 0xfffff000 points to Page Directory (= PDE #0)
  */
  pd_entry_add_attribute(&directory->entries[1023], PDE_PRESENT);
  pd_entry_add_attribute(&directory->entries[1023], PDE_WRITABLE);
  pd_entry_set_frame(&directory->entries[1023], (physical_addr)directory);

  // switch to our page directory
  vmm_switch_directory(directory);

  // enable paging
  pmm_enable_paging(true);
}

page_index vmm_virt_to_index(virtual_addr address)
{
  page_index index;

  //align address to 4k (highest 20-bits of address)
  address &= ~0xfff;

  index.page_directory = address >> 22; // each page table covers 0x400000 bytes in memory
  index.page_table = (address << 10) >> 22;

  return index;
}

void vmm_pdirectory_clear(pdirectory *directory)
{
  memset(directory, 0, sizeof(pdirectory));
}

void vmm_ptable_clear(ptable *table)
{
  memset(table, 0, sizeof(ptable));
}

physical_addr vmm_get_phys_addr(uint32_t addr)
{
  return addr & ~0xfff;
}

static void page_fault_handler(ir_params *params)
{
  asm volatile("sti");

  // Gather fault info and print to screen
  uint32_t faulting_addr;

  asm volatile("mov %%cr2, %0"
               : "=r"(faulting_addr));

  kprintf("\nPage fault while accessing address: 0x%x\n\n", faulting_addr);

  uint32_t present = params->err_code & PAGING_ERR_PRESENT;
  uint32_t rw = params->err_code & PAGING_ERR_RW;
  uint32_t user = params->err_code & PAGING_ERR_USER;

  // https://wiki.osdev.org/Paging#Handling
  if (!user && !rw && !present)
  {
    kprint("Supervisory process tried to read a non-present page entry.\n");
  }

  if (!user && !rw && present)
  {
    kprint("Supervisory process tried to read a page and caused a protection fault.\n");
  }

  if (!user && rw && !present)
  {
    kprint("Supervisory process tried to write to a non-present page entry.\n");
  }

  if (!user && rw && present)
  {
    kprint("Supervisory process tried to write a page and caused a protection fault.\n");
  }

  if (user && !rw && !present)
  {
    kprint("User process tried to read a non-present page entry.\n");
  }

  if (user && !rw && present)
  {
    kprint("User process tried to read a page and caused a protection fault.\n");
  }

  if (user && rw && !present)
  {
    kprint("User process tried to write to a non-present page entry.\n");
  }

  if (user && rw && present)
  {
    kprint("User process tried to write a page and caused a protection fault.\n");
  }

  kernel_panic("");
}
