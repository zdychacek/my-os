#include "lib/memory.h"

void memcpy(void *dest, const void *src, size_t n)
{
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  for (size_t i = 0; i < n; i++)
  {
    cdest[i] = csrc[i];
  }
}

void memset(void *dest, int val, size_t n)
{
  uint8_t *temp = (uint8_t *)dest;

  for (; n != 0; n--)
    *temp++ = val;
}
