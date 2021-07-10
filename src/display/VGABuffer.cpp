#include <display/VGABuffer.h>
#include <memory/PhysicalMemoryManager.h>
#include <lib/String.h>

namespace Display
{
  VGABuffer::VGABuffer()
  {
    m_buffer = (Char *)(PhysicalMemoryManager::GetInstance()->ConvertPhysicalAddressToVirtual(0xb8000));
  }

  void VGABuffer::ClearRow(size_t row)
  {
    Char empty = {
      character : ' ',
      color : m_currentColor,
    };

    for (size_t col = 0; col < MaxColumns; col++)
    {
      m_buffer[col + MaxColumns * row] = empty;
    }
  }

  void VGABuffer::Clear()
  {
    for (size_t i = 0; i < MaxRows; i++)
    {
      ClearRow(i);
    }
  }

  void VGABuffer::WriteNewline()
  {
    m_currentColumn = 0;

    if (m_currentRow < MaxRows - 1)
    {
      m_currentRow++;
      return;
    }

    for (size_t row = 1; row < MaxRows; row++)
    {
      for (size_t col = 0; col < MaxColumns; col++)
      {
        Char character = m_buffer[col + MaxColumns * row];

        m_buffer[col + MaxColumns * (row - 1)] = character;
      }
    }

    ClearRow(MaxRows - 1);
  }

  void VGABuffer::Write(char character)
  {
    if (character == '\n')
    {
      WriteNewline();
      return;
    }

    if (m_currentColumn > MaxColumns)
    {
      WriteNewline();
    }

    m_buffer[m_currentColumn + MaxColumns * m_currentRow] = {
      character : (uint8_t)character,
      color : m_currentColor,
    };

    m_currentColumn++;
  }

  void VGABuffer::Write(const char *string, ...)
  {
    auto handle = m_spinlock.makeHandle();

    char buffer[1024] = {-1};

    va_list args;
    va_start(args, string);

    std::String::PrintInto(buffer, string, args);

    va_end(args);

    uint8_t *character = (uint8_t *)buffer;

    while (*character)
    {
      Write(*character++);
    }
  }

  void VGABuffer::SetColor(uint8_t foreground, uint8_t background)
  {
    m_currentColor = foreground + (background << 4);
  }
}
