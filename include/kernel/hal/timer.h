#pragma once

#include "lib/types.h"

#define PIT_DATA_0 0x40
#define PIT_DATA_1 0x41
#define PIT_DATA_2 0x42
#define PIT_COMMAND 0x43
#define PIT_REPEATING_MODE 0x36

void timer_init(uint32_t freq);
