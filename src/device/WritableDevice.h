#pragma once

namespace Device
{
  class WritableDevice
  {
  public:
    WritableDevice() {}
    virtual ~WritableDevice() {}

    virtual void Write(const char *str, ...) = 0;
  };
}
