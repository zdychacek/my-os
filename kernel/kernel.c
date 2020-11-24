#include "kernel.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../drivers/ata.h"
#include "../drivers/rtc.h"
#include "../lib/string.h"
#include "../lib/mem.h"

void _start()
{
  // TODO: get kernel base/end from bootloader
  memory_init(0x10000);
  memory_print_info();

  clear_screen();

  isr_init();
  irq_init();
  rtc_init();

  kprint("You can type following commands: \n"
         "- END to halt the CPU\n"
         "- ALLOC to request a memory from malloc()\n"
         "- READ to read a data from disk\n"
         "- TIME to print current date\n"
         "- MEM to print memory info\n"
         "> ");
}

void user_input(char *input)
{
  if (strcmp(input, "END") == 0)
  {
    kprint("Stopping the CPU. Bye!\n");
    asm volatile("hlt");
  }
  else if (strcmp(input, "ALLOC") == 0)
  {
    char *data = (char *)malloc(16);

    free(data);
  }
  else if (strcmp(input, "READ") == 0)
  {
    uint32_t *data = (uint32_t *)malloc(sizeof(uint32_t) * 128);

    // read the first sector from the disk
    read_sectors_ATA_PIO(data, 0, 1);

    kprintf("%x ", data[127] & 0xFF);
    kprintf("%x ", (data[127] >> 8) & 0xFF);
    kprintf("%x ", (data[127] >> 16) & 0xFF);
    kprintf("%x\n", (data[127] >> 24) & 0xFF);

    free(data);
  }
  else if (strcmp(input, "TIME") == 0)
  {
    rtc_print_time();
  }
  else if (strcmp(input, "MEM") == 0)
  {
    memory_print_info();
  }

  kprint("\n> ");
}
