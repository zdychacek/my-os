#include "../drivers/screen.h"
#include "util.h"
#include "../cpu/isr.h"
#include "../cpu/idt.h"

void _start()
{
  clear_screen();

  isr_install();

  /* Test the interrupts */
  asm volatile("int $2");
  asm volatile("int $3");
}
