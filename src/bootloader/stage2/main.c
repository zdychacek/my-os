#include "bootloader/bootinfo.h"
#include "bootloader/stage2/elf.h"
#include "bootloader/stage2/ext2.h"
#include "bootloader/stage2/vga.h"
#include "bootloader/stage2/bootconfig.h"
#include "bootloader/stage2/lib.h"
#include "lib/types.h"
#include "lib/string.h"

uint32_t HEAP_START = 0x200000;
uint32_t HEAP;

void stage2_main(multiboot_info *mbi)
{
  HEAP = HEAP_START;

  char buf[32];

  //clear the screen
  vga_clear();
  vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
  lsroot();

  vga_pretty("Memory map:\n", VGA_LIGHTGREEN);

  // get memory size in KB
  uint32_t phys_memory_avail = 1024 + mbi->memory_lo + mbi->memory_hi * 64;

  vga_pretty("Physical memory available ", VGA_LIGHTMAGENTA);
  itoa(phys_memory_avail, buf, 10);
  vga_pretty(buf, VGA_LIGHTMAGENTA);
  vga_puts("\n");

  memory_region *region = (memory_region *)mbi->mmap_addr;
  size_t regions_count = mbi->mmap_length / sizeof(memory_region);

  for (size_t i = 0; i < regions_count; i++)
  {
    itoa(region[i].base, buf, 16);
    vga_puts(buf);
    vga_putc('-');

    itoa(region[i].base + region[i].len, buf, 16);
    vga_puts(buf);
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
    itoa(region[i].len / 1024, buf, 10);
    vga_puts(buf);
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
