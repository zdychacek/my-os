#include "rtc.h"
#include "../drivers/screen.h"
#include "../cpu/ports.h"

uint8_t bcd_decimal(uint8_t hex)
{
  uint8_t dec = ((hex & 0xF0) >> 4) * 10 + (hex & 0x0F);

  return dec;
}

uint8_t rtc_get_year()
{
  port_byte_write(0x70, 0x09);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_month()
{
  port_byte_write(0x70, 0x08);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_day()
{
  port_byte_write(0x70, 0x07);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_weekday()
{
  port_byte_write(0x70, 0x06);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_hour()
{
  port_byte_write(0x70, 0x04);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_minute()
{
  port_byte_write(0x70, 0x02);
  return bcd_decimal(port_byte_read(0x71));
}

uint8_t rtc_get_second()
{
  port_byte_write(0x70, 0x00);
  return bcd_decimal(port_byte_read(0x71));
}

date_time_t rtc_get_time()
{
  date_time_t current_date_time = {
      .second = rtc_get_second(),
      .minute = rtc_get_minute(),
      .hour = rtc_get_hour(),
      .day = rtc_get_day(),
      .weekday = rtc_get_weekday(),
      .month = rtc_get_month(),
      .year = rtc_get_year()};

  return current_date_time;
}

void rtc_print_time()
{
  date_time_t current_time = rtc_get_time();

  kprintf("%d.%d.%d %d:%d:%d\n",
          current_time.day, current_time.month, current_time.year,
          current_time.hour, current_time.minute, current_time.second);
}

void rtc_init()
{
  kprintf("RTC initialized...\n");
}
