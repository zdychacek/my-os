#include "bootloader/bootinfo.h"
#include "kernel/main.h"
#include "kernel/hal/timer.h"
#include "kernel/hal/isr.h"
#include "kernel/hal/rtc.h"
#include "kernel/drivers/display.h"
#include "kernel/drivers/ata.h"
#include "kernel/drivers/keyboard.h"
#include "kernel/memory/heap.h"
#include "kernel/memory/memory_region.h"
#include "kernel/memory/pm_manager.h"
#include "lib/string.h"
#include "lib/memory.h"

//#define PRINT_MEMORY_MAP

extern uint32_t kernel_start;
extern uint32_t kernel_end;

void kmain(unsigned long magic, multiboot_info *mbi)
{
  display_init();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    kprintf("Invalid magic number: 0x%x\n", magic);
    return;
  }

  kprintf("Multiboot flags = 0x%x\n", mbi->flags);

  if (mbi->flags != MULTIBOOT_FLAGS)
  {
    kprintf("Bad multiboot flags (expecting: 0x%x, got: 0x%x)\n", MULTIBOOT_FLAGS, mbi->flags);
    return;
  }

  kprintf("Boot device = 0x%x\n", mbi->boot_device);

  // get memory size in KB
  uint32_t phys_memory_avail = 1024 + mbi->memory_lo + mbi->memory_hi * 64;

  pmm_init(phys_memory_avail, (uint32_t)&kernel_end);
  kprintf("(PMM) Initialized with %d KB\n", phys_memory_avail);

  memory_region *region = (memory_region *)mbi->mmap_addr;
  size_t regions_count = mbi->mmap_length / sizeof(memory_region);

#ifdef PRINT_MEMORY_MAP
  kprint("(PMM) Memory map:\n");
#endif

  for (size_t i = 0; i < regions_count; i++)
  {
    char type = (char)region[i].type;

    if (type == 1)
    {
      pmm_init_region(region[i].base, region[i].len);
    }
#ifdef PRINT_MEMORY_MAP
    kprintf("0x%x-0x%x %s %d KB\n",
            (uint32_t)region[i].base,
            (uint32_t)(region[i].base + region[i].len),
            type == 1 ? "Usable RAM" : type == 2 ? "Reserved" : "N/A",
            region[i].len / 1024);
#endif
  }

  // exclude kernel
  pmm_deinit_region((uint32_t)&kernel_start, (size_t)&kernel_end);

  kprintf("(PMM) Allocation blocks count: %d (block size: %d bytes)\n",
          pmm_get_block_count(), pmm_get_block_size());
  kprintf("(PMM) Free allocation blocks count: %d\n", pmm_get_free_block_count());

  //memory_init((uint32_t)&kernel_end);ok
  isr_init();
  timer_init(50);
  keyboard_init();
  rtc_init();
  // enable interrupts
  irq_init();

  kprintf("Kernel begins at 0x%x, ends at 0x%x\n\n", &kernel_start, &kernel_end);

  kprint("   _____            ________     _________\n"
         "  /     \\   ___.__. \\_____  \\   /   _____/\n"
         " /  \\ /  \\ <   |  |  /   |   \\  \\_____  \\ \n"
         "/    Y    \\ \\___  | /    |    \\ /        \\ \n"
         "\\____|__  / / ____| \\_______  //_______  /\n"
         "        \\/  \\/              \\/         \\/ \n");

  kprint("\nType command or HELP: \n> ");

  for (;;)
    ;
}

void user_input(char *input)
{
  if (strcmp(input, "HELP") == 0)
  {
    kprint("\nYou can type following commands: \n\n"
           "- END to halt the CPU\n"
           "- ALLOC to request a memory from malloc()\n"
           "- READ to read a data from disk\n"
           "- TIME to print current date\n"
           "- MEM to print memory info\n");
  }
  else if (strcmp(input, "END") == 0)
  {
    kprint("Stopping the CPU. Bye!\n");
    asm volatile("hlt");
  }
  // else if (strcmp(input, "ALLOC") == 0)
  // {
  //   char *data = (char *)kmalloc(16);

  //   kfree(data);
  // }
  // else if (strcmp(input, "READ") == 0)
  // {
  //   uint32_t *data = (uint32_t *)kmalloc(sizeof(uint32_t) * 128);

  //   // read the first sector from the disk
  //   read_sectors_ATA_PIO(data, 0, 1);

  //   kprintf("%x ", data[127] & 0xFF);
  //   kprintf("%x ", (data[127] >> 8) & 0xFF);
  //   kprintf("%x ", (data[127] >> 16) & 0xFF);
  //   kprintf("%x\n", (data[127] >> 24) & 0xFF);

  //   kfree(data);
  // }
  else if (strcmp(input, "TIME") == 0)
  {
    rtc_print_time();
  }
  else if (strcmp(input, "MEM") == 0)
  {
    memory_print_info();
  }
  else
  {
    kprintf("Unknown command \"%s\".\n", input);
  }

  kprint("\n> ");
}
