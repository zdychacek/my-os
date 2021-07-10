#pragma once

#include <types.h>

namespace SMP
{
  class Spinlock
  {
  public:
    class Handle
    {
    private:
      Spinlock *m_lock;

    protected:
      // allow the outer class access to this "hidden" constructor
      friend class Spinlock;
      Handle(Spinlock *lock) : m_lock(lock)
      {
        m_lock->acquire();
      }

    public:
      Handle(Handle &other) = delete;
      Handle(Handle &&other) = default;

      ~Handle()
      {
        m_lock->release();
      }
    };

  private:
    uint64_t m_lock;

  public:
    Spinlock();

    // NOTE: these should not be used by external code.
    void acquire();
    void release();

    Handle makeHandle()
    {
      return Handle(this);
    }
  };
}
