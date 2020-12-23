#pragma once

#define kernel_panic(...)                                                                             \
  {                                                                                                   \
    kprintf("\n***KERNEL PANIC*** in %s at line %d in function: %s\n", __FILE__, __LINE__, __func__); \
    kprintf(__VA_ARGS__);                                                                             \
    while (1)                                                                                         \
      ;                                                                                               \
  }
