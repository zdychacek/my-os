#pragma once

#include <stdint.h>
#include <stddef.h>

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

// Sometimes we want to keep parameters to a function for later use
// and this is a solution to avoid the 'unused parameter' compiler warning
#define unused(x) (void)(x)
