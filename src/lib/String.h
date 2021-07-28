#pragma once

#include <types.h>
#include <lib/Array.h>

namespace std
{
  class String
  {
  private:
    Array<char> m_content;

  public:
    String() {}
    String(const char *s);

    const char *cStr() const;
    size_t Length() const;

    static size_t PrintInto(char *outBuffer, const char *message, va_list args);
    static size_t PrintInto(char *str, const char *message, ...);
    static char *Format(const char *message, ...);
    static size_t Length(const char *string);
    static size_t Length(String &string);
  };
}
