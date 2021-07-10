#include <cxx/abi.h>

extern "C"
{
  __ExitEntry __exitEntries[AtExitMaxFunctions];
  uarch_t __exitEntryCount = 0;

  int __cxa_atexit(AtExitFunction exitFunction, void *parameter, void *dso)
  {
    if (__exitEntryCount >= AtExitMaxFunctions)
    {
      return -1;
    };

    __exitEntries[__exitEntryCount++] = {exitFunction, parameter, dso};

    return 0;
  };

  void __cxa_finalize(void *dso)
  {
    // From the itanium abi, https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor-runtime-api
    //
    // When `__cxa_finalize(dso)` is called, it should walk the termination function list, calling each in turn
    // if `dso` matches `exitEntry.dso` for the termination function entry. If `dso == nullptr`, it should call all of them.
    // Multiple calls to `__cxa_finalize` shall not result in calling termination function entries multiple times;
    // the implementation may either remove entries or mark them finished.

    int entryIndex = __exitEntryCount;

    while (--entryIndex >= 0)
    {
      auto &exitEntry = __exitEntries[entryIndex];
      bool needsCalling = exitEntry.destructor && (!dso || dso == exitEntry.dso);

      if (needsCalling)
      {
        exitEntry.destructor(exitEntry.parameter);
        exitEntry.destructor = nullptr;
      }
    }
  }

  int __cxa_guard_acquire(__guard *guard)
  {
    return !*(char *)(guard);
  }

  void __cxa_guard_release(__guard *guard)
  {
    *(char *)guard = 1;
  }

  void __cxa_guard_abort(__guard *)
  {
  }

  void __cxa_pure_virtual()
  {
    // Do nothing (https://wiki.osdev.org/C++).
  }
}
