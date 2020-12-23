#include "bootloader/bootinfo.h"
#include "bootloader/stage2/elf.h"
#include "bootloader/stage2/ext2.h"
#include "bootloader/stage2/vga.h"
#include "bootloader/stage2/bootconfig.h"
#include "bootloader/stage2/lib.h"
#include "common/common.h"
#include <stddef.h>

uint32_t HEAP_START = 0x200000;
uint32_t HEAP;

void stage2_main(multiboot_info *mbi)
{
  HEAP = HEAP_START;

  //clear the screen
  vga_clear();
  vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
  lsroot();

  vga_pretty("Memory map:\n", VGA_LIGHTGREEN);

  // get memory size in KB
  uint32_t phys_memory_avail = 1024 + mbi->memory_lo + mbi->memory_hi * 64;

  vga_pretty("Physical memory available ", VGA_LIGHTMAGENTA);
  vga_pretty(itoa(phys_memory_avail, 10), VGA_LIGHTMAGENTA);
  vga_puts("\n");

  memory_region *region = (memory_region *)mbi->mmap_addr;
  size_t regions_count = mbi->mmap_length / sizeof(memory_region);

  for (size_t i = 0; i < regions_count; i++)
  {
    vga_puts(itoa(region[i].base, 16));
    vga_putc('-');
    vga_puts(itoa(region[i].base + region[i].len, 16));
    vga_putc('\t');

    switch ((char)region[i].type)
    {
    case 1:
    {
      vga_puts("Usable Ram\t");

      break;
    }
    case 2:
    {
      vga_puts("Reserved\t");
      break;
    }
    default:
      vga_putc('\n');
    }
    vga_puts(itoa(region[i].len / 1024, 10));
    vga_puts(" KB\n");
  }

  int boot_conf_inode = ext2_find_child("boot.conf", 2);

  char *config_text = ext2_read_file(ext2_inode(1, boot_conf_inode));

  bootconfig *boot_cfg = parse_config(config_text);

  elf_load(mbi, boot_cfg);

  free(boot_cfg);

  for (;;)
    ;
}
