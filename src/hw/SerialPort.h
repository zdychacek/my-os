#pragma once

#include <types.h>
#include <smp/Spinlock.h>
#include <device/WritableDevice.h>

namespace hw
{
  class SerialPort : public Device::WritableDevice
  {
  public:
    enum PortAddress : uint16_t
    {
      COM1 = 0x3F8,
      COM2 = 0x2F8,
      COM3 = 0x3E8,
      COM4 = 0x2E8,
    };

    SerialPort(PortAddress address);
    void Write(const char *ch, ...);
    void Write(char ch);
    char Read();

  private:
    SMP::Spinlock m_spinlock;

    enum IOPort : uint16_t
    {
      DataRegister = 0,
      InterruptEnableRegister = 1,
      DivisorLow = 0,
      DivisorHigh = 1,
      InterruptIdentification = 2,
      LineControlRegister = 3,
      ModemControlRegister = 4,
      LineStatusRegister = 5,
      ModemStatusRegister = 6,
      ScratchRegister = 7
    };
    uint16_t m_portAddress;
  };
}
