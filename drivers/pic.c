#include "pic.h"
#include "ports.h"

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
  // save masks
  u8 a1 = port_byte_read(PIC1_DATA);
  u8 a2 = port_byte_read(PIC2_DATA);

  // starts the initialization sequence (in cascade mode)
  port_byte_write(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  port_byte_write(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  port_byte_write(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  port_byte_write(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  port_byte_write(PIC1_DATA, 4);       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  port_byte_write(PIC2_DATA, 2);       // ICW3: tell Slave PIC its cascade identity (0000 0010)
  port_byte_write(PIC1_DATA, ICW4_8086);
  port_byte_write(PIC2_DATA, ICW4_8086);

  // restore saved masks
  port_byte_write(PIC1_DATA, a1);
  port_byte_write(PIC2_DATA, a2);
}

void PIC_sendEOI(u8 int_num)
{
  if (int_num >= 40)
    port_byte_write(PIC2_COMMAND, PIC_EOI);

  port_byte_write(PIC1_COMMAND, PIC_EOI);
}

void IRQ_set_mask(u8 IRQline)
{
  u16 port;
  u8 value;

  if (IRQline < 8)
  {
    port = PIC1_DATA;
  }
  else
  {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = port_byte_read(port) | (1 << IRQline);
  port_byte_write(port, value);
}

void IRQ_clear_mask(u8 IRQline)
{
  u16 port;
  u8 value;

  if (IRQline < 8)
  {
    port = PIC1_DATA;
  }
  else
  {
    port = PIC2_DATA;
    IRQline -= 8;
  }
  value = port_byte_read(port) & ~(1 << IRQline);
  port_byte_write(port, value);
}
