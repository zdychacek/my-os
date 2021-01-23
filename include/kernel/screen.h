#pragma once

#include "lib/types.h"
#include "bootloader/bootinfo.h"

void screen_init(vbe_mode_info *mode_info);
void screen_draw_pixel(int x, int y, uint32_t color);
void screen_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void screen_draw_rect(int x, int y, int width, int height, uint32_t color);
