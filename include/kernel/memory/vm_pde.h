#pragma once

#include "lib/types.h"
#include "kernel/memory/pm_manager.h"

enum PAGE_PDE_FLAGS
{
  I86_PDE_PRESENT = 1,        // 0000000000000000000000000000001
  I86_PDE_WRITABLE = 2,       // 0000000000000000000000000000010
  I86_PDE_USER = 4,           // 0000000000000000000000000000100
  I86_PDE_PWT = 8,            // 0000000000000000000000000001000
  I86_PDE_PCD = 0x10,         // 0000000000000000000000000010000
  I86_PDE_ACCESSED = 0x20,    // 0000000000000000000000000100000
  I86_PDE_DIRTY = 0x40,       // 0000000000000000000000001000000
  I86_PDE_4MB = 0x80,         // 0000000000000000000000010000000
  I86_PDE_CPU_GLOBAL = 0x100, // 0000000000000000000000100000000
  I86_PDE_LV4_GLOBAL = 0x200, // 0000000000000000000001000000000
  I86_PDE_FRAME = 0x7FFFF000  // 1111111111111111111000000000000
};

// a page directery entry
typedef uint32_t pd_entry;

// sets a flag in the page table entry
extern void pd_entry_add_attribute(pd_entry *entry, uint32_t attribute);

// clears a flag in the page table entry
extern void pd_entry_remove_attribute(pd_entry *entry, uint32_t attribute);

// sets a frame to page table entry
extern void pd_entry_set_frame(pd_entry *entry, physical_addr address);

// get page table entry frame address
extern physical_addr pd_entry_get_frame(pd_entry entry);

// test if page is present
extern bool pd_entry_is_present(pd_entry entry);

// test if directory is user mode
extern bool pd_entry_is_user(pd_entry entry);

// test if directory contains 4mb pages
extern bool pd_entry_is_4mb(pd_entry entry);

// test if page is writable
extern bool pd_entry_is_writable(pd_entry entry);
