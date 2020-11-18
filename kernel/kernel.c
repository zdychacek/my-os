#include "../cpu/isr.h"
#include "../drivers/screen.h"

void _start()
{
  clear_screen();

  isr_install();
  irq_install();
}
