#include <debug/Logger.h>
#include <display/VGABuffer.h>
#include <lib/String.h>

namespace Debug
{
  const char *Logger::m_logLevelToStrings[5] = {
      "TRACE",
      "LOG",
      "WARN",
      "ERR",
      "FATAL",
  };

  Logger::Logger(Device::WritableDevice *device)
      : m_device(device)
  {
  }

  void Logger::WriteInternal(LogLevel level, const char *prefix, const char *msg, va_list args)
  {
    auto handle = m_spinlock.makeHandle();
    char buffer[1024];

    std::String::PrintInto(buffer, msg, args);

    m_device->Write("[%s] %s %s\n", MapLogLevelToString(level), prefix, buffer);
  }

  void Logger::Write(LogLevel level, const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(level, prefix, msg, args);
  }

  void Logger::Trace(const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(LogLevel::LevelTrace, prefix, msg, args);
  }

  void Logger::Log(const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(LogLevel::LevelLog, prefix, msg, args);
  }

  void Logger::Warning(const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(LogLevel::LevelWarning, prefix, msg, args);
  }

  void Logger::Error(const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(LogLevel::LevelError, prefix, msg, args);
  }

  void Logger::Fatal(const char *prefix, const char *msg, ...)
  {
    va_list args;
    va_start(args, msg);
    va_end(args);

    WriteInternal(LogLevel::LevelFatal, prefix, msg, args);
  }

  const char *Logger::MapLogLevelToString(LogLevel level)
  {
    // TODO: check array bounds
    return m_logLevelToStrings[level - 1];
  }
}
