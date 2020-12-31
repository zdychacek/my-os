#include "bootloader/bootinfo.h"
#include "kernel/main.h"
#include "kernel/hal/timer.h"
#include "kernel/hal/idt.h"
#include "kernel/hal/gdt.h"
#include "kernel/hal/rtc.h"
#include "kernel/drivers/display.h"
#include "kernel/drivers/ata.h"
#include "kernel/drivers/keyboard.h"
#include "kernel/memory/heap.h"
#include "kernel/memory/memory_region.h"
#include "kernel/memory/pm_manager.h"
#include "kernel/memory/vm_manager.h"
#include "lib/string.h"
#include "lib/memory.h"

// defined and exported from linker script
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t kernel_virt_start;
extern uint32_t kernel_virt_end;

void kmain(unsigned long magic, multiboot_info *mbi)
{
  display_init();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    kprintf("Invalid bootloader magic number: 0x%x\n", magic);

    return;
  }

  kprintf("Multiboot flags = 0x%x\n", mbi->flags);

  // TODO: fix use bitmask check
  if (mbi->flags != MULTIBOOT_FLAGS)
  {
    kprintf("Bad multiboot flags (expecting: 0x%x, got: 0x%x)\n", MULTIBOOT_FLAGS, mbi->flags);

    return;
  }

  kprintf("Boot device = 0x%x\n", mbi->boot_device);

  // get memory size in KB
  uint32_t phys_memory_avail = 1024 + mbi->memory_lo + mbi->memory_hi * 64;

  pmm_init(phys_memory_avail, (physical_addr)&kernel_virt_start, (physical_addr)&kernel_virt_end,
           (memory_region *)mbi->mmap_addr, mbi->mmap_length);

  vmm_init();

  //memory_init((uint32_t)&kernel_end);
  gdt_init();
  idt_init();
  timer_init(50);
  keyboard_init();
  rtc_init();
  // enable interrupts
  irq_init();

  kprintf("Kernel begins at 0x%x (0x%x), ends at 0x%x (0x%x)\n\n",
          &kernel_start, &kernel_virt_start, &kernel_end, &kernel_virt_end);

  kprint("   _____            ________     _________\n"
         "  /     \\   ___.__. \\_____  \\   /   _____/\n"
         " /  \\ /  \\ <   |  |  /   |   \\  \\_____  \\ \n"
         "/    Y    \\ \\___  | /    |    \\ /        \\ \n"
         "\\____|__  / / ____| \\_______  //_______  /\n"
         "        \\/  \\/              \\/         \\/ \n");

  kprint("\nType command or HELP: \n> ");

  // Uncomment to raise Page Fault exception
  // int *data = (int *)0x900000;
  // *data = 12;

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
