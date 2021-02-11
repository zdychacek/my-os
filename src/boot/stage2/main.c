#include "boot/bootinfo.h"
#include "boot/stage2/elf.h"
#include "boot/stage2/ext2.h"
#include "boot/stage2/vga.h"
#include "boot/stage2/bootconfig.h"
#include "boot/stage2/lib.h"
#include "lib/types.h"
#include "lib/string.h"

uint32_t HEAP_START = 0x200000;
uint32_t HEAP;

void stage2_main(multiboot_info *mbi)
{
  HEAP = HEAP_START;

  //clear the screen
  vga_clear();
  vga_pretty("Stage2 loaded...\n", VGA_LIGHTGREEN);
  lsroot();

  int boot_conf_inode = ext2_find_child("boot.conf", 2);

  char *config_text = ext2_read_file(ext2_inode(1, boot_conf_inode));

  bootconfig *boot_cfg = parse_config(config_text);

  elf_load(mbi, boot_cfg);

  free(boot_cfg);

  for (;;)
    ;
}
