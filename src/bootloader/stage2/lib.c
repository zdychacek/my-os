#include "bootloader/stage2/lib.h"

extern uint32_t HEAP;

void *malloc(size_t n)
{
  return (void *)(HEAP += n);
}

void free(void *mem)
{
  unused(mem);
}

uint8_t inb(uint16_t port)
{
  // "=a" (result) means: put AL register in variable result when finished
  // "d" (_port) means: load EDX with _port
  unsigned char result;

  asm volatile("inb %1, %0"
               : "=a"(result)
               : "dN"(port));

  return result;
}

void outb(uint16_t port, uint16_t data)
{
  asm("out %%al, %%dx"
      :
      : "a"(data), "d"(port));
}

void insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl"
               : "=D"(addr), "=c"(cnt)
               : "d"(port), "0"(addr), "1"(cnt)
               : "memory", "cc");
}