#pragma once

#include <types.h>
#include <smp/Spinlock.h>
#include <device/WritableDevice.h>
#include <utils/Singleton.h>
#include <interfaces/NonCopyable.h>

using namespace Utils;
namespace Display
{
  static constexpr size_t MaxColumns = 80;
  static constexpr size_t MaxRows = 25;

  class VGABuffer : private Interfaces::NonCopyable,
                    public Singleton<VGABuffer>,
                    public Device::WritableDevice
  {
    friend class SingletonFactory<VGABuffer>; // to access the private constructor
  public:
    enum Color
    {
      Black = 0,
      Blue = 1,
      Green = 2,
      Cyan = 3,
      Red = 4,
      Magenta = 5,
      Brown = 6,
      LightGray = 7,
      DarkGray = 8,
      LightBlue = 9,
      LightGreen = 10,
      LightCyan = 11,
      LightRed = 12,
      Pink = 13,
      Yellow = 14,
      White = 15,
    };

  private:
    struct Char
    {
      uint8_t character;
      uint8_t color;
    };

    SMP::Spinlock m_spinlock;
    Char *m_buffer;
    size_t m_currentColumn = 0;
    size_t m_currentRow = 0;
    uint8_t m_currentColor = Color::White | Color::Black << 4;

    VGABuffer();
    void ClearRow(size_t row);
    void WriteNewline();

  public:
    void Clear();
    void Write(char ch);
    void Write(const char *string, ...);
    void SetColor(uint8_t foreground, uint8_t background);
  };
}
