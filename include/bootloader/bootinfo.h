#pragma once

#include "lib/types.h"

typedef struct _multiboot_info
{
  uint32_t flags;
  uint32_t memory_lo;
  uint32_t memory_hi;
  uint32_t boot_device;
  uint32_t cmd_line;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms0;
  uint32_t syms1;
  uint32_t syms2;
  uint32_t mmap_length;
  uint32_t mmap_addr;
  uint32_t drives_length;
  uint32_t drives_addr;
  uint32_t config_table;
  uint32_t bootloader_name;
  uint32_t apm_table;
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint32_t vbe_interface_addr;
  uint16_t vbe_interface_len;
} __attribute__((packed)) multiboot_info;

typedef struct _vbe_mode_info
{
  uint16_t width;
  uint16_t height;
  uint32_t framebuffer;
  uint16_t pitch;
  uint8_t bpp;
  uint16_t bytes_per_pixel;
} __attribute__((packed)) vbe_mode_info;

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Multiboot flags
#define MULTIBOOT_FLAGS_MEM 1
#define MULTIBOOT_FLAGS_BOOTDEVICE 1 << 2
#define MULTIBOOT_FLAGS_MMAP 1 << 7
#define MULTIBOOT_FLAGS_VBE 1 << 12
