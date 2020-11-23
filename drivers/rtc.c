#include "rtc.h"
#include "../drivers/screen.h"
#include "../cpu/ports.h"

uint8_t rtc_get_year()
{
  port_byte_write(0x70, 0x09);
  return port_byte_read(0x71);
}

uint8_t rtc_get_month()
{
  port_byte_write(0x70, 0x08);
  return port_byte_read(0x71);
}

uint8_t rtc_get_day()
{
  port_byte_write(0x70, 0x07);
  return port_byte_read(0x71);
}

uint8_t rtc_get_weekday()
{
  port_byte_write(0x70, 0x06);
  return port_byte_read(0x71);
}

uint8_t rtc_get_hour()
{
  port_byte_write(0x70, 0x04);
  return port_byte_read(0x71);
}

uint8_t rtc_get_minute()
{
  port_byte_write(0x70, 0x02);
  return port_byte_read(0x71);
}

uint8_t rtc_get_second()
{
  port_byte_write(0x70, 0x00);
  return port_byte_read(0x71);
}

void rtc_print_time()
{
  kprintf("%x.%x.20%x %x:%x:%x\n",
          rtc_get_day(), rtc_get_month(), rtc_get_year(),
          rtc_get_hour(), rtc_get_minute(), rtc_get_second());
}

void rtc_install()
{
  rtc_print_time();
  kprint("RTC initialized...\n");
}
