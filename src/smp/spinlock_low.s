global spinlock_acquire
global spinlock_release

; https://wiki.osdev.org/Spinlock

spinlock_acquire:
  lock bts qword [rdi], 0 ; Attempt to acquire the lock (in case lock is uncontended)
  jc .spin_with_pause
  ret

.spin_with_pause:
  pause ; Tell CPU we're spinning
  test qword [rdi], 1 ; Is the lock free?
  jnz .spin_with_pause ; no, wait
  jmp spinlock_acquire ; retry

spinlock_release:
  ; mark as 0; if this is qword-aligned, the access will be atomic.
  mov qword [rdi], 0
  ret
