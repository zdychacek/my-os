#pragma once

#include <types.h>

void _AssertInternal(const char *expressionString, bool expression, const char *fileName, size_t lineNumber, const char *functionName, const char *msg);

#ifndef NDEBUG
#define Assert(expr, msg) \
  _AssertInternal(#expr, expr, __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION(), msg)
#else
#define Assert(expr, msg) ;
#endif
