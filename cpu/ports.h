#pragma once

#include <stdint.h>

uint8_t port_byte_read(uint16_t port);
void port_byte_write(uint16_t port, uint8_t data);

uint16_t port_word_read(uint16_t port);
void port_word_write(uint16_t port, uint16_t data);

uint32_t port_long_read(uint32_t port);
void port_long_write(uint32_t port, uint32_t data);
