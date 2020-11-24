#pragma once

#include <stdint.h>

typedef struct
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t weekday;
  uint8_t month;
  uint16_t year;
} date_time_t;

void rtc_init();
void rtc_print_time();
date_time_t rtc_get_time();
