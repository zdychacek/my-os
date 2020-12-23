#pragma once

#define NULL (void *)0

typedef unsigned size_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef unsigned char *va_list;

// width of stack == width of int
#define STACKITEM int

// round up width of objects pushed on stack. The expression before the
// & ensures that we get 0 for objects of size 0.
#define VA_SIZE(TYPE) \
    ((sizeof(TYPE) + sizeof(STACKITEM) - 1) & ~(sizeof(STACKITEM) - 1))

// &(LASTARG) points to the LEFTMOST argument of the function call (before the ...)
#define va_start(AP, LASTARG) \
    (AP = ((va_list) & (LASTARG) + VA_SIZE(LASTARG)))

// nothing for va_end
#define va_end(AP)

#define va_arg(AP, TYPE) \
    (AP += VA_SIZE(TYPE), *((TYPE *)(AP - VA_SIZE(TYPE))))

// Sometimes we want to keep parameters to a function for later use
// and this is a solution to avoid the 'unused parameter' compiler warning
#define unused(x) (void)(x)

#define bool int
#define true 1
#define false 0
