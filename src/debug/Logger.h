#pragma once

#include <smp/Spinlock.h>
#include <hw/SerialPort.h>
#include <device/WritableDevice.h>
#include <utils/Singleton.h>
#include <utils/SingletonFactory.h>
#include <interfaces/NonCopyable.h>

using namespace Utils;
using namespace Device;

namespace Debug
{
  class Logger : private Interfaces::NonCopyable, public Singleton<Logger>
  {
    friend class SingletonFactory<Logger, WritableDevice *>; // to access the private constructor

  public:
    enum LogLevel
    {
      LevelTrace = 1,
      LevelLog = 2,
      LevelWarning = 3,
      LevelError = 4,
      LevelFatal = 5,
    };

  private:
    SMP::Spinlock m_spinlock;
    WritableDevice *m_device;

    Logger(WritableDevice *device);
    static const char *m_logLevelToStrings[5];
    const char *MapLogLevelToString(LogLevel level);
    void WriteInternal(LogLevel level, const char *prefix, const char *msg, va_list args);

  public:
    void Write(LogLevel level, const char *prefix, const char *msg, ...);
    void Trace(const char *prefix, const char *msg, ...);
    void Log(const char *prefix, const char *msg, ...);
    void Warning(const char *prefix, const char *msg, ...);
    void Error(const char *prefix, const char *msg, ...);
    void Fatal(const char *prefix, const char *msg, ...);
  };
}
