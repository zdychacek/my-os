#include "../drivers/screen.h"
#include "util.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"

void _start()
{
  clear_screen();

  isr_install();

  asm volatile("sti");
  init_timer(50);
}
