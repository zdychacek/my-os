#pragma once

#include "lib/types.h"

#define VGA_COLS 80
#define VGA_ROWS 25

#define VGA_BLACK 0x00
#define VGA_BLUE 0x01
#define VGA_GREEN 0x02
#define VGA_CYAN 0x03
#define VGA_RED 0x04
#define VGA_MAGENTA 0x05
#define VGA_BROWN 0x06
#define VGA_LIGHTGREY 0x07
#define VGA_DARKGREY 0x08
#define VGA_LIGHTBLUE 0x09
#define VGA_LIGHTGREEN 0x0A
#define VGA_LIGHTCYAN 0x0B
#define VGA_LIGHTRED 0x0C
#define VGA_LIGHTMAGENTA 0x0D
#define VGA_LIGHTBROWN 0x0E
#define VGA_WHITE 0x0F

#define VGA_COLOR(f, b) ((b << 4) | (f & 0xF))
#define RGB(r, g, b) (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF))

void vga_move_cursor(uint16_t x, uint16_t y);
void vga_update_cursor();
int vga_current_x();
int vga_current_y();
void vga_setcolor(int color);
void vga_clear();
void vga_scroll();
void vga_overwrite_color(int color, int start_x, int start_y, int end_x, int end_y);
void vga_kputc(char c, int x, int y);
void vga_kputs(char *s, int x, int y);
void vga_puts(char *s);
void vga_putc(char c);
void vga_pretty(char *s, int color);
void vga_pretty_assert(char *s, int color);
void printx(char *msg, uint32_t i);
