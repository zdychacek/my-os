#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

uint8_t port_byte_read(uint16_t port);
void port_byte_write(uint16_t port, uint8_t data);
uint16_t port_word_read(uint16_t port);
void port_word_write(uint16_t port, uint16_t data);

#endif
