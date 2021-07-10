#pragma once

#include <types.h>

constexpr uint8_t AtExitMaxFunctions = 128;

extern "C"
{
  typedef unsigned uarch_t;
  __extension__ typedef int __guard __attribute__((mode(__DI__)));
  void *__dso_handle;

  typedef void (*AtExitFunction)(void *);

  struct __ExitEntry
  {
    /*
    * Each member is at least 4 bytes large. Such that each entry is 12bytes.
    * 128 * 12 = 1.5KB exact.
    */
    AtExitFunction destructor;
    void *parameter;
    void *dso;
  };

  int __cxa_atexit(AtExitFunction exitFunction, void *parameter, void *dso);
  void __cxa_finalize(void *dsoHandle);

  int __cxa_guard_acquire(__guard *guard);
  void __cxa_guard_release(__guard *guard);
  void __cxa_guard_abort(__guard *guard);
  void __cxa_pure_virtual();
};
