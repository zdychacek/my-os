#pragma once

#include "lib/types.h"
#include "kernel/memory/pm_manager.h"

enum PAGE_PTE_FLAGS
{
  PTE_PRESENT = 1,          // 0000000000000000000000000000001
  PTE_WRITABLE = 2,         // 0000000000000000000000000000010
  PTE_USER = 4,             // 0000000000000000000000000000100
  PTE_WRITETHOUGH = 8,      // 0000000000000000000000000001000
  PTE_NOT_CACHEABLE = 0x10, // 0000000000000000000000000010000
  PTE_ACCESSED = 0x20,      // 0000000000000000000000000100000
  PTE_DIRTY = 0x40,         // 0000000000000000000000001000000
  PTE_PAT = 0x80,           // 0000000000000000000000010000000
  PTE_CPU_GLOBAL = 0x100,   // 0000000000000000000000100000000
  PTE_LV4_GLOBAL = 0x200,   // 0000000000000000000001000000000
  PTE_FRAME = 0x7FFFF000    // 1111111111111111111000000000000
};

// page table entry
typedef uint32_t pt_entry;

// sets a flag in the page table entry
extern void pt_entry_add_attribute(pt_entry *entry, uint32_t attribute);

// clears a flag in the page table entry
extern void pt_entry_remove_attribute(pt_entry *entry, uint32_t attribute);

// sets a frame to page table entry
extern void pt_entry_set_frame(pt_entry *entry, physical_addr address);

// get page table entry frame address
extern physical_addr pt_entry_get_frame(pt_entry entry);

// test if page is present
extern bool pt_entry_is_present(pt_entry entry);

// test if page is writable
extern bool pt_entry_is_writable(pt_entry entry);
