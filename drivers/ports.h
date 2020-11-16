#ifndef PORTS_H
#define PORTS_H

#include "../cpu/types.h"

u8 port_byte_read(u16 port);
void port_byte_write(u16 port, u8 data);
u16 port_word_read(u16 port);
void port_word_write(u16 port, u16 data);

#endif
