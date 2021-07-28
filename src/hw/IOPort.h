#pragma once

#include <types.h>

namespace hw
{
  class IOPort
  {
  private:
    const uint16_t m_address;

  public:
    IOPort(uint16_t address) : m_address(address) {}

    uint8_t Read8();
    void Write(uint8_t value);
  };
}
