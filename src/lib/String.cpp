#include <lib/String.h>

static void PrintHexNumber(unsigned int value, unsigned int width, char *buf, int *ptr);
static void PrintDecNumber(unsigned int value, unsigned int width, char *buf, int *ptr);

namespace std
{
  String::String(const char *s)
  {
    size_t length = 0;

    while (s[length])
    {
      length++;
    }

    m_content.Resize(length + 1);

    for (size_t i = 0; i <= length; i++)
    {
      m_content[i] = s[i];
    }
  }

  const char *String::cStr() const
  {
    return &m_content[0];
  }

  size_t String::Length() const
  {
    return m_content.Size();
  }

  size_t String::PrintInto(char *outBuffer, const char *message, va_list args)
  {
    int ptr = 0;
    size_t messageLength = Length(message);

    for (size_t i = 0; i < messageLength && message[i]; ++i)
    {
      if (message[i] != '%')
      {
        outBuffer[ptr++] = message[i];
        continue;
      }
      i++;

      size_t argWidth = 0;

      while (message[i] >= '0' && message[i] <= '9')
      {
        argWidth *= 10;
        argWidth += message[i] - '0';
        ++i;
      }

      // message[i] == '%'
      switch (message[i])
      {
      // String
      case 's':
      {
        auto string = va_arg(args, char *);

        while (*string)
        {
          outBuffer[ptr++] = *string++;
        }
        break;
      }
      // Single character
      case 'c':
        outBuffer[ptr++] = (char)va_arg(args, int);
        break;
      // Hexadecimal number
      case 'x':
        PrintHexNumber(va_arg(args, unsigned int), argWidth, outBuffer, &ptr);
        break;
      // Decimal number
      case 'd':
        PrintDecNumber(va_arg(args, unsigned int), argWidth, outBuffer, &ptr);
        break;
      // Escape
      case '%':
        outBuffer[ptr++] = '%';
        break;
      // Nothing at all, just dump it
      default:
        outBuffer[ptr++] = message[i];
        break;
      }
    }

    // Ensure the buffer ends with null
    outBuffer[ptr] = '\0';

    return ptr;
  }

  size_t String::PrintInto(char *str, const char *message, ...)
  {
    va_list args;
    va_start(args, message);
    va_end(args);

    return String::PrintInto(str, message, args);
  }

  size_t String::Length(const char *string)
  {
    size_t index = 0;

    while (string[index] != '\0')
    {
      ++index;
    }

    return index;
  }

  size_t String::Length(String &string)
  {
    return string.Length();
  }
}

/*
 * Hexadecimal number to string.
 */
static void PrintHexNumber(unsigned int value, unsigned int width, char *buf, int *ptr)
{
  int i = width;

  if (i == 0)
  {
    i = 8;
  }

  unsigned int n_width = 1;
  unsigned int j = 0xf;

  while (value > j && j < UINT32_MAX)
  {
    n_width += 1;
    j *= 0x10;
    j += 0xf;
  }

  while (i > (int)n_width)
  {
    buf[*ptr] = '0';
    *ptr += 1;
    i--;
  }

  i = (int)n_width;
  while (i-- > 0)
  {
    buf[*ptr] = "0123456789abcdef"[(value >> (i * 4)) & 0xf];
    *ptr += +1;
  }
}

/*
 * Decimal number to string.
 */
static void PrintDecNumber(unsigned int value, unsigned int width, char *buf, int *ptr)
{
  unsigned int n_width = 1;
  unsigned int i = 9;

  while (value > i && i < UINT32_MAX)
  {
    n_width += 1;
    i *= 10;
    i += 9;
  }

  int printed = 0;
  while (n_width + printed < width)
  {
    buf[*ptr] = '0';
    *ptr += 1;
    printed += 1;
  }

  i = n_width;
  while (i > 0)
  {
    unsigned int n = value / 10;
    int r = value % 10;

    buf[*ptr + i - 1] = r + '0';
    i--;
    value = n;
  }
  *ptr += n_width;
}
