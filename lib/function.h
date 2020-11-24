#pragma once

// Sometimes we want to keep parameters to a function for later use
// and this is a solution to avoid the 'unused parameter' compiler warning */
#define UNUSED(x) (void)(x)

#define panic(...)                                                                                  \
  {                                                                                                 \
    kprintf("***KERNEL PANIC*** in %s at line %d in function: %s\n", __FILE__, __LINE__, __func__); \
    kprintf(__VA_ARGS__);                                                                           \
    while (1)                                                                                       \
      ;                                                                                             \
  }
