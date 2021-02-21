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
#include "kernel/panic.h"
#include "kernel/screen.h"
#include "lib/string.h"

// defined and exported from linker script
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t kernel_virt_start;
extern uint32_t kernel_virt_end;

void kmain(unsigned long magic, multiboot_info *mbi)
{
  // TODO: deprecated, will be deleted
  display_init();

  // vbe_mode_info *mode_info = (vbe_mode_info *)mbi->vbe_mode_info;

  // init screen
  // screen_init(mode_info);

  kprintf("magic: %x\n", magic);
  kprintf("mbi: %x\n", (uint32_t)mbi);
  kprintf("flags: %x\n", (uint32_t)mbi->flags);
  kprintf("MULTIBOOT_FLAGS_BOOTDEVICE: %x\n", (uint32_t)mbi->flags & MULTIBOOT_FLAGS_BOOTDEVICE);
  kprintf("MULTIBOOT_FLAGS_MEM: %x\n", (uint32_t)mbi->flags & MULTIBOOT_FLAGS_MEM);
  kprintf("MULTIBOOT_FLAGS_MMAP: %x\n", (uint32_t)mbi->flags & MULTIBOOT_FLAGS_MMAP);

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    kernel_panic("Invalid bootloader magic number: 0x%x\n", magic);
  }

  kprint("Multiboot magic number OK\n");

  if (!((mbi->flags & MULTIBOOT_FLAGS_BOOTDEVICE) &&
        (mbi->flags & MULTIBOOT_FLAGS_MEM) &&
        (mbi->flags & MULTIBOOT_FLAGS_MMAP) /*&&
        (mbi->flags & MULTIBOOT_FLAGS_VBE)*/
        ))
  {
    kernel_panic("Bad multiboot flags (got: 0x%x)\n", mbi->flags);
  }

  kprint("Multiboot flags OK\n");

  kprintf("Boot device = 0x%x\n", mbi->boot_device);

  int rect_size = 25;

  // screen_draw_rect(0, 0, rect_size, rect_size, 0xff0000);
  // screen_draw_rect(mode_info->width - rect_size, 0, rect_size, rect_size, 0x00ff00);
  // screen_draw_rect(mode_info->width - rect_size, mode_info->height - rect_size, rect_size, rect_size, 0xffff00);
  // screen_draw_rect(0, mode_info->height - rect_size, rect_size, rect_size, 0x00ffff);
  // screen_draw_rect(400, 400, 150, 150, 0xdeadbeef);

  // screen_draw_line(100, 100, 300, 300, 0xff0000);
  // screen_draw_line(300, 100, 100, 300, 0x0000ff);

  // get memory size in KB
  uint32_t phys_memory_avail = 1024 + mbi->memory_lo + mbi->memory_hi * 64;

  // init physical memory manager
  pmm_init(phys_memory_avail, (physical_addr)&kernel_virt_start, (physical_addr)&kernel_virt_end,
           (memory_region *)mbi->mmap_addr, mbi->mmap_length);

  // initi virtual memory manager and reload Page Directory
  vmm_init();

  // init dynamic memory allocator
  heap_init(HEAP_VIRT_START, HEAP_VIRT_END);

  // reload Global Descriptor Table
  gdt_init();

  // reload Interrupt Descriptor Table
  idt_init();

  // init timer service
  timer_init(50);

  // init keyboard input
  keyboard_init();

  // init clock service
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

  ptable *frame = (ptable *)pmm_alloc_frame();

  vmm_map_page((physical_addr)frame, 0xeeeeeeee);

  uint32_t *data = (uint32_t *)0xeeeeeeee;
  *data = 28;

  kprintf("data: %d\n", *data);

  vmm_unmap_page(0xeeeeeeee);

  // uncomment to raise Page Fault Exception
  // *data = 13;
  // kprintf("data: %d\n", *data);

  // char *a = malloc(3 * sizeof(char));
  // heap_dump();
  // kprint("-------------------\n");

  // char *b = malloc(20 * sizeof(char));
  // heap_dump();
  // kprint("-------------------\n");

  // free(a);
  // heap_dump();
  // kprint("-------------------\n");

  // free(b);
  // heap_dump();
  // kprint("-------------------\n");

  while (true)
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
  else if (strcmp(input, "ALLOC") == 0)
  {
    char *data = (char *)malloc(1000);

    unused(data);

    // free(data);

    heap_dump();
    kprint("-------------------\n");
  }
  else if (strcmp(input, "READ") == 0)
  {
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * 512);

    // read the first sector from the disk
    read_sectors_ATA_PIO((uint32_t *)data, 0, 1);

    // TODO: fix disk reading
    kprintf("%x ", data[510]);
    kprintf("%x ", data[511]);

    free(data);
  }
  else if (strcmp(input, "TIME") == 0)
  {
    rtc_print_time();
  }
  else if (strcmp(input, "MEM") == 0)
  {
    kprintf("heap free space: %d bytes\n", heap_get_free_space());
    kprintf("heap used space: %d bytes\n", heap_get_used_space());
    kprintf("frames free count: %d\n", pmm_get_free_frames_count());
    kprintf("frames used count: %d\n", pmm_get_used_frames_count());
  }
  else
  {
    kprintf("Unknown command \"%s\".\n", input);
  }

  kprint("\n> ");
}
