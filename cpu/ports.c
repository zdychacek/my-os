#include "ports.h"
#include "../cpu/type.h"

/**
 * Read a byte from the specified port
 */
u8 port_byte_read(u16 port)
{
  u8 result;
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

void port_byte_write(u16 port, u8 data)
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

u16 port_word_read(u16 port)
{
  u16 result;
  asm("in %%dx, %%ax"
      : "=a"(result)
      : "d"(port));
  return result;
}

void port_word_write(u16 port, u16 data)
{
  asm("out %%ax, %%dx"
      :
      : "a"(data), "d"(port));
}
