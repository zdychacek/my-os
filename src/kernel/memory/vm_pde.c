#include "kernel/memory/vm_pde.h"

inline void pd_entry_add_attribute(pd_entry *entry, uint32_t attrib)
{
  *entry |= attrib;
}

inline void pd_entry_remove_attribute(pd_entry *entry, uint32_t attrib)
{
  *entry &= ~attrib;
}

inline void pd_entry_set_frame(pd_entry *entry, physical_addr addr)
{
  *entry = (*entry & ~PDE_FRAME) | addr;
}

inline physical_addr pd_entry_get_frame(pd_entry entry)
{
  return entry & PDE_FRAME;
}

inline bool pd_entry_is_present(pd_entry entry)
{
  return entry & PDE_PRESENT;
}

inline bool pd_entry_is_writable(pd_entry entry)
{
  return entry & PDE_WRITABLE;
}

inline bool pd_entry_is_user(pd_entry entry)
{
  return entry & PDE_USER;
}

inline bool pd_entry_is_4mb(pd_entry entry)
{
  return entry & PDE_4MB;
}
