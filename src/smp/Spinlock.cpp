#include <smp/Spinlock.h>

extern "C"
{
  extern void spinlock_acquire(uint64_t *lock);
  extern void spinlock_release(uint64_t *release);
}
namespace SMP
{
  Spinlock::Spinlock()
  {
    m_lock = 0;
  }

  void Spinlock::acquire()
  {
    spinlock_acquire(&m_lock);
  }

  void Spinlock::release()
  {
    spinlock_release(&m_lock);
  }
}
