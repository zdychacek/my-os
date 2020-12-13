#include "boot/stage2/elf.h"
#include "boot/stage2/ext2.h"
#include "boot/stage2/vga.h"
#include "boot/stage2/lib.h"
#include "boot/stage2/bootconfig.h"
#include "common/common.h"

uint32_t HEAP_START = 0x200000;
uint32_t HEAP;

void stage2_main(uint32_t *mem_info)
{
  HEAP = HEAP_START;

  //clear the screen
  vga_clear();
  vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
  lsroot();

  mmap *mem_map = (mmap *)*mem_info;
  mmap *max = (mmap *)*++mem_info;

  memcpy((void *)0x000F1000, mem_map, (uint32_t)max - (uint32_t)mem_map);

  vga_pretty("Memory map:\n", VGA_LIGHTGREEN);

  while (mem_map < max)
  {

    vga_puts(itoa(mem_map->base, 16));
    vga_putc('-');
    vga_puts(itoa(mem_map->base + mem_map->len, 16));
    vga_putc('\t');

    switch ((char)mem_map->type)
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
    vga_puts(itoa(mem_map->len / 0x400, 10));
    vga_puts(" KB\n");
    mem_map++;
  }

  int boot_conf_inode = ext2_find_child("boot.conf", 2);

  char *config_text = ext2_read_file(ext2_inode(1, boot_conf_inode));

  bootconfig *boot_cfg = parse_config(config_text);

  elf_load(mem_map, boot_cfg);

  free(boot_cfg);

  for (;;)
    ;
}
