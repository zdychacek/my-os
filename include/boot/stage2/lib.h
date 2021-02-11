#pragma once

#include "lib/types.h"
#include "boot/stage2/vga.h"

#define assert(e) ((e) ? (void)0 : vga_pretty_assert(#e, VGA_RED))

void *malloc(size_t n);
void free(void *mem);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint16_t data);
void insl(int port, void *addr, int cnt);
