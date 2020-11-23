#include "kernel.h"
#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../drivers/ata.h"
#include "../drivers/rtc.h"
#include "../libc/string.h"
#include "../libc/mem.h"

void _start()
{
  clear_screen();

  isr_install();
  irq_install();
  rtc_install();

  uint32_t *data = (uint32_t *)kmalloc(512, 1, NULL);

  read_sectors_ATA_PIO(data, 0, 1);

  int i = 0;

  while (i < 128)
  {
    kprintf("%x ", data[i] & 0xFF);
    kprintf("%x ", (data[i] >> 8) & 0xFF);
    kprintf("%x ", (data[i] >> 16) & 0xFF);
    kprintf("%x ", (data[i] >> 24) & 0xFF);
    i++;
  }

  kprint("Type something, it will go through the kernel\n"
         "Type END to halt the CPU or PAGE to request a kmalloc()\n> ");
}

void user_input(char *input)
{
  if (strcmp(input, "END") == 0)
  {
    kprint("Stopping the CPU. Bye!\n");
    asm volatile("hlt");
  }
  else if (strcmp(input, "PAGE") == 0)
  {
    // kmalloc test
    uint32_t phys_addr;
    uint32_t page = kmalloc(1000, 1, &phys_addr);

    char page_str[16] = "";
    hex_to_ascii(page, page_str);
    char phys_str[16] = "";
    hex_to_ascii(phys_addr, phys_str);

    kprint("Page: ");
    kprint(page_str);
    kprint(", physical address: ");
    kprint(phys_str);
    kprint("\n");
  }

  kprint("You said: ");
  kprint(input);
  kprint("\n> ");
}
