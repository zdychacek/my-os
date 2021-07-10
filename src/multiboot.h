#pragma once

#include <types.h>

// the multiboot magic number
constexpr uint32_t MultibootMagic = 0x36d76289;

// the multiboot info tag numbers
enum MultibootTagType
{
  Terminator = 0,
  Cmdline = 1,
  BootLoader = 2,
  Module = 3,
  Memory = 4,
  BootDevice = 5,
  MemoryMap = 6,
  VBE = 7,
  Framebuffer = 8,
  ELF = 9,
  APM = 10
};

// indicates the type of a memory region
enum MemoryMapType
{
  Available = 1,
  Reserved = 2,
  ACPIReclaimable = 3,
  ACPINVS = 4,
  Bad = 5
};

// align the given address up to the next 8 byte boundary
#define MULTIBOOT_ALIGN(x) (((x) + 7) & 0xFFFFFFFFFFFFFFF8)

// multiboot information structure
struct __attribute__((__packed__)) Multiboot
{
  uint32_t totalSize;
  uint32_t reserved;
};

struct __attribute__((__packed__)) MultibootMemoryMapEntry
{
  uint64_t baseAddress;
  uint64_t length;
  uint32_t type;
  uint32_t reserved;
};

// multiboot tag structure
struct __attribute__((__packed__)) MultibootTag
{
  // general information
  uint32_t type;
  uint32_t size;

  // type-specific information
  union types
  {
    // mmap tag
    struct __attribute__((__packed__)) MemoryMap
    {
      uint32_t entrySize;
      uint32_t entryVersion;
      // entries follow here
    };

    // module tag
    struct __attribute__((__packed__)) Module
    {
      uint32_t moduleStart;
      uint32_t moduleEnd;
      char string[1];
    };

    // cmdline tag
    struct __attribute__((__packed__)) Cmdline
    {
      char string[1];
    };

    // elf tag
    struct __attribute__((__packed__)) Elf
    {
      uint32_t size;
      uint16_t sh_num;
      uint16_t sh_entsize;
      uint16_t sh_shstrndx;
      uint16_t reserved;
      char data[1];
    };
  };
};
