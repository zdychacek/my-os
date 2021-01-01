#include "kernel/hal/gdt.h"
#include "lib/string.h"

typedef struct _gdtr
{
  uint16_t limit; // size of gdt
  uint32_t base;  // base address of gdt
} __attribute__((packed)) gdtr;

// global descriptor table is an array of descriptors
static gdt_descriptor _gdt[MAX_DESCRIPTORS];

// gdtr data
static gdtr _gdtr;

// install gdtr
static void gdt_install()
{
  asm volatile("lgdt (%0)"
               :
               : "r"(&_gdtr));
}

// Setup a descriptor in the Global Descriptor Table
void gdt_set_descriptor(uint32_t index, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand)
{
  if (index > MAX_DESCRIPTORS)
  {
    return;
  }

  // null out the descriptor
  memset((void *)&_gdt[index], 0, sizeof(gdt_descriptor));

  // set limit and base addresses
  _gdt[index].base_lo = (uint16_t)(base & 0xffff);
  _gdt[index].base_mid = (uint8_t)((base >> 16) & 0xff);
  _gdt[index].base_hi = (uint8_t)((base >> 24) & 0xff);
  _gdt[index].limit = (uint16_t)(limit & 0xffff);

  // set flags and grandularity bytes
  _gdt[index].flags = access;
  _gdt[index].grand = (uint8_t)((limit >> 16) & 0x0f);
  _gdt[index].grand |= grand & 0xf0;
}

// returns descriptor in gdt
gdt_descriptor *gdt_get_descriptor(int index)
{
  if (index > MAX_DESCRIPTORS)
  {
    return 0;
  }

  return &_gdt[index];
}

void gdt_init()
{
  // set up gdtr
  _gdtr.limit = (sizeof(gdt_descriptor) * MAX_DESCRIPTORS) - 1;
  _gdtr.base = (uint32_t)&_gdt[0];

  // set null descriptor
  gdt_set_descriptor(0, 0, 0, 0, 0);

  // set default code descriptor
  gdt_set_descriptor(1, 0, 0xffffffff,
                     GDT_DESC_READWRITE | GDT_DESC_EXEC_CODE | GDT_DESC_CODEDATA | GDT_DESC_MEMORY,
                     GDT_GRAND_4K | GDT_GRAND_32BIT | GDT_GRAND_LIMITHI_MASK);

  // set default data descriptor
  gdt_set_descriptor(2, 0, 0xffffffff,
                     GDT_DESC_READWRITE | GDT_DESC_CODEDATA | GDT_DESC_MEMORY,
                     GDT_GRAND_4K | GDT_GRAND_32BIT | GDT_GRAND_LIMITHI_MASK);

  gdt_install();
}
