#include <hw/IOPort.h>

namespace hw
{
  uint8_t IOPort::Read8()
  {
    uint8_t result;

    __asm__ __volatile__("in (%%dx), %%al"
                         : "=a"(result)
                         : "d"(m_address));
    return result;
  }

  void IOPort::Write(uint8_t value)
  {
    __asm__ __volatile__("outb %%al, (%%dx)"
                         :
                         : "a"(value), "d"(m_address));
  }
}
