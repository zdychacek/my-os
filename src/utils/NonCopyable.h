#pragma once

namespace Utils
{
  class NonCopyable
  {
  public:
    NonCopyable() {}

  protected:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
  };
}
