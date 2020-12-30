#include "kernel/memory/vm_pte.h"

inline void pt_entry_add_attribute(pt_entry *entry, uint32_t attribute)
{
  *entry |= attribute;
}

inline void pt_entry_remove_attribute(pt_entry *entry, uint32_t attribute)
{
  *entry &= ~attribute;
}

inline void pt_entry_set_frame(pt_entry *entry, physical_addr address)
{
  *entry = (*entry & ~I86_PTE_FRAME) | address;
}

inline physical_addr pt_entry_get_frame(pt_entry entry)
{
  return entry & I86_PTE_FRAME;
}

inline bool pt_entry_is_present(pt_entry entry)
{
  return entry & I86_PTE_PRESENT;
}

inline bool pt_entry_is_writable(pt_entry entry)
{
  return entry & I86_PTE_WRITABLE;
}
