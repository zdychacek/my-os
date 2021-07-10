#pragma once

namespace Utils
{
  class NonMovable
  {
  public:
    NonMovable() {}

  protected:
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
  };
}
