#pragma once

namespace Interfaces
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
