#pragma once

#include "lib/types.h"

// maximum amount of descriptors allowed
#define MAX_DESCRIPTORS 3

// set access bit
#define GDT_DESC_ACCESS 0x0001 //00000001

// descriptor is readable and writable. default: read only
#define GDT_DESC_READWRITE 0x0002 //00000010

// set expansion direction bit
#define GDT_DESC_EXPANSION 0x0004 //00000100

// executable code segment. Default: data segment
#define GDT_DESC_EXEC_CODE 0x0008 //00001000

// set code or data descriptor. defult: system defined descriptor
#define GDT_DESC_CODEDATA 0x0010 //00010000

// set dpl bits
#define GDT_DESC_DPL 0x0060 //01100000

// set "in memory" bit
#define GDT_DESC_MEMORY 0x0080 //10000000

// masks out limitHi (High 4 bits of limit)
#define GDT_GRAND_LIMITHI_MASK 0x0f //00001111

// set os defined bit
#define GDT_GRAND_OS 0x10 //00010000

// set if 32bit. default: 16 bit
#define GDT_GRAND_32BIT 0x40 //01000000

// 4k grandularity. default: none
#define GDT_GRAND_4K 0x80 //10000000

typedef struct _gdt_descriptor
{
  uint16_t limit; // bits 0-15 of segment limit
  // bits 0-23 of base address
  uint16_t base_lo;
  uint8_t base_mid;
  uint8_t flags; // descriptor access flags
  uint8_t grand;
  uint8_t base_hi; // bits 24-32 of base address
} __attribute__((packed)) gdt_descriptor;

// setup a descriptor in the Global Descriptor Table
extern void gdt_set_descriptor(uint32_t index, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand);

// returns descritor
extern gdt_descriptor *gdt_get_descriptor(int index);

// initializes gdt
extern void gdt_init();
