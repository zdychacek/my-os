#include <debug/Assert.h>
#include <debug/Logger.h>
#include <hw/cpu.h>
#include <lib/String.h>

using namespace std;
using namespace Debug;
using namespace hw;

void _AssertInternal(const char *expressionString,
                     bool expression,
                     const char *fileName,
                     size_t lineNumber,
                     const char *functionName,
                     const char *msg)
{
  if (expression)
  {
    return;
  }

  char buffer[256];

  String::PrintInto(buffer, " Message: %s\n  Expected: %s\n  Source: %s:%d in function %s", msg, expressionString, fileName, lineNumber, functionName);

  Logger::GetInstance()->Fatal("Assertion failed!!!\n", buffer);

  cpu::HangForever();
}
