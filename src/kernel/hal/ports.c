#include "kernel/hal/ports.h"

/**
 * Read a byte from the specified port
 */
uint8_t port_byte_read(uint16_t port)
{
  uint8_t result;
  /* Inline assembler syntax
     * !! Notice how the source and destination registers are switched from NASM !!
     *
     * '"=a" (result)'; set '=' the C variable '(result)' to the value of register e'a'x
     * '"d" (port)': map the C variable '(port)' into e'd'x register
     *
     * Inputs and outputs are separated by colons
     */
  asm("in %%dx, %%al"
      : "=a"(result)
      : "d"(port));

  return result;
}

void port_byte_write(uint16_t port, uint8_t data)
{
  /* Notice how here both registers are mapped to C variables and
     * nothing is returned, thus, no equals '=' in the asm syntax
     * However we see a comma since there are two variables in the input area
     * and none in the 'return' area
     */
  asm("out %%al, %%dx"
      :
      : "a"(data), "d"(port));
}

uint16_t port_word_read(uint16_t port)
{
  uint16_t result;

  asm("in %%dx, %%ax"
      : "=a"(result)
      : "d"(port));

  return result;
}

void port_word_write(uint16_t port, uint16_t data)
{
  asm("out %%ax, %%dx"
      :
      : "a"(data), "d"(port));
}

uint32_t port_long_read(uint32_t port)
{
  uint32_t result;

  asm("inl %%dx,%%eax"
      : "=a"(result)
      : "d"(port));

  return result;
}

void port_long_write(uint32_t port, uint32_t data)
{
  asm("outl %%eax,%%dx" ::"d"(port), "a"(data));
};
