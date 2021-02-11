
#include "boot/stage2/vga.h"
#include "boot/stage2/lib.h"
#include "lib/string.h"

int current_x = 0;
int current_y = 0;
int current_attrib = VGA_COLOR(VGA_WHITE, VGA_BLACK);

char *VGA_MEMORY = (char *)0x000B8000;

int vga_current_x() { return current_x; }
int vga_current_y() { return current_y; }

// Move the screen's cursor to the specified pos and line x is the char position, y is the line
void vga_move_cursor(uint16_t x, uint16_t y)
{
  unsigned temp = (y * VGA_COLS) + x;

  outb(0x3D4, 14);
  outb(0x3D5, temp >> 8);
  outb(0x3D4, 15);
  outb(0x3D5, temp);
}

void vga_update_cursor()
{
  vga_move_cursor(current_x / 2, current_y);
}

void vga_setcolor(int color)
{
  current_attrib = color;
}

void vga_clear()
{
  char *vga_address = VGA_MEMORY;
  const long size = VGA_COLS * VGA_ROWS;

  for (long i = 0; i < size; i++)
  {
    *vga_address++ = 0;              // character value
    *vga_address++ = current_attrib; // color value
  }
  current_y = 0;
  current_x = 0;
}

/* Scroll the screen up one line */
void vga_scroll()
{
  if (current_y >= VGA_ROWS)
  {
    uint8_t *vga_addr = (uint8_t *)VGA_MEMORY;
    uint8_t temp = current_y - (VGA_ROWS - 1);
    memcpy(vga_addr, vga_addr + temp * 160, (VGA_ROWS - temp) * 160 * 2);
    current_y = VGA_ROWS - 1;
  }
  vga_update_cursor();
}

void vga_overwrite_color(int color, int start_x, int start_y, int end_x, int end_y)
{
  unused(end_y);

  char *vga_address = VGA_MEMORY;
  int sizeend = 2 * end_x + (160 * start_y);

  for (int i = (start_x + 1 + (160 * start_y)); i < sizeend; i += 2)
  {

    vga_address[i] = color;
  }
}

// kernel level putc, designate x and y position
void vga_kputc(char c, int x, int y)
{
  char *vga_address = VGA_MEMORY + (x + y * 160);

  if (isascii(c))
  {
    *vga_address = c | (current_attrib << 8);
  }
}

void vga_kputs(char *s, int x, int y)
{
  while (*s != 0)
  {
    vga_kputc(*s, x += 2, y);
    s++;
  }
}

void vga_puts(char *s)
{
  while (*s != 0)
  {
    vga_putc(*s);
    unused(*s++);
  }
}

// Automatically update text position, used in vga_puts
void vga_putc(char c)
{
  if (c == '\n')
  {
    current_y += 1;
    current_x = 0;
    return;
  }

  if (c == '\b')
  {
    current_x -= 2;
    vga_kputc(' ', current_x, current_y);
    return;
  }

  if (c == '\t')
  {
    while (current_x % 16)
      current_x++;
    current_x += 2;

    if (current_x >= 160)
    {
      current_x = 0;
      current_y += 1;
    }
    return;
  }
  vga_kputc(c, current_x, current_y);
  current_x += 2;

  if (current_x >= 160)
  {
    current_x = 0;
    current_y += 1;
  }
  vga_scroll();
}

void vga_pretty(char *s, int color)
{
  int start_x = current_x;
  int start_y = current_y;

  vga_puts(s);

  vga_overwrite_color(color, start_x, start_y, VGA_COLS, current_y);
}

void vga_pretty_assert(char *s, int color)
{
  int start_x = current_x;
  int start_y = current_y;

  vga_puts("Assertion error: ");
  vga_puts(s);
  vga_puts("\n");

  vga_overwrite_color(color, start_x, start_y, VGA_COLS, current_y);
}

void printx(char *msg, uint32_t i)
{
  char buf[32];

  vga_puts(msg);
  vga_puts(itoa(i, buf, 16));
  vga_putc('\n');
}
