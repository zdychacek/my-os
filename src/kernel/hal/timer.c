#include "kernel/hal/idt.h"
#include "kernel/hal/ports.h"
#include "kernel/hal/timer.h"
#include "kernel/drivers/display.h"

uint32_t tick = 0;

static void timer_callback(ir_params *regs)
{
  tick++;
  unused(regs);
}

void timer_init(uint32_t freq)
{
  // Install the function we just wrote
  idt_install_ir_handler(IRQ0, timer_callback);

  // Get the PIT value: hardware clock at 1193180 Hz
  uint32_t divisor = 1193180 / freq;
  uint8_t low = (uint8_t)(divisor & 0xFF);
  uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

  // Send the command
  port_byte_write(PIT_COMMAND, PIT_REPEATING_MODE);
  port_byte_write(PIT_DATA_0, low);
  port_byte_write(PIT_DATA_0, high);

  kprintf("Timer is ticking at frequency %d Hz\n", freq);
}
