#include <hw/SerialPort.h>
#include <hw/IOPort.h>
#include <lib/String.h>
#include <smp/Spinlock.h>
#include <debug/Logger.h>

namespace hw
{
  SerialPort::SerialPort(PortAddress address)
  {
    m_portAddress = address;

    // Disable all interrupts
    IOPort(m_portAddress + Register::InterruptEnable).Write(0);
    // Enable DLAB (set baud rate divisor)
    IOPort(m_portAddress + Register::LineControl).Write(0x80);
    // Set divisor to 3 (low byte) 38400 baud
    IOPort(m_portAddress + Register::DivisorLow).Write(0x03);
    // Set divisor high byte to 0
    IOPort(m_portAddress + Register::DivisorLow).Write(0);
    // 8 bits, no parity, one stop bit
    IOPort(m_portAddress + Register::LineControl).Write(0x03);
    // Enable FIFO, clear them, with 14-byte threshold
    IOPort(m_portAddress + Register::InterruptIdentification).Write(0xc7);
    // IRQs enabled, RTS/DSR set
    IOPort(m_portAddress + Register::ModemControl).Write(0x0b);
  }

  void SerialPort::Write(char ch)
  {
    IOPort status(m_portAddress + Register::LineStatus);

    // while TX buffer full
    while ((status.Read8() & 0x20) == 0)
    {
    }

    IOPort(m_portAddress).Write((uint8_t)ch);
  }

  void SerialPort::Write(const char *string, ...)
  {
    auto handle = m_spinlock.makeHandle();
    char buffer[1024];

    va_list args;
    va_start(args, string);
    va_end(args);

    std::String::PrintInto(buffer, string, args);

    uint8_t *character = (uint8_t *)buffer;

    while (*character)
    {
      Write(*character++);
    }
  }

  char SerialPort::Read()
  {
    IOPort status(m_portAddress + Register::LineStatus);

    // while RX buffer full
    while ((status.Read8() & 1) == 0)
    {
    }

    return IOPort(m_portAddress).Read8();
  }
}
