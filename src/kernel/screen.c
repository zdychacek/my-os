#include "kernel/screen.h"
#include "kernel/memory/defs.h"
#include "lib/math.h"

typedef struct _screen
{
  uint16_t width;
  uint16_t height;
  uint8_t bpp;
  uint16_t pitch;
  uint32_t *framebuffer;
} screen_t;

static screen_t screen;

void screen_init(vbe_mode_info *mode_info)
{
  screen.framebuffer = (uint32_t *)FRAMEBUFFER_VIRT_START;
  screen.bpp = mode_info->bpp;
  screen.width = mode_info->width;
  screen.height = mode_info->height;
  screen.pitch = mode_info->pitch;
}

void screen_draw_pixel(int x, int y, uint32_t color)
{
  uint32_t *pixel_offset = (uint32_t *)(y * screen.pitch + (x * (screen.bpp / 8)) + (uint32_t)screen.framebuffer);

  *pixel_offset = color;
}

void screen_draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  while (true)
  {
    screen_draw_pixel(x0, y0, color);

    if (x0 == x1 && y0 == y1)
    {
      break;
    }
    e2 = err;

    if (e2 > -dx)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void screen_draw_rect(int x, int y, int width, int height, uint32_t color)
{
  for (int i = x; i < x + width; i++)
  {
    for (int j = y; j < y + height; j++)
    {
      screen_draw_pixel(i, j, color);
    }
  }
}
