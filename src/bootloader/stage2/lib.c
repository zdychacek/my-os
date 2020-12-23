#include <stdint.h>
#include "bootloader/stage2/lib.h"
#include "common/common.h"

// Returns length of a null-terminated string
size_t strlen(const char *s)
{
  const char *p = s;
  size_t i = 0;
  while (*p++ != 0)
    i++;
  return i;
}

// Appends a copy of the source string to the destination string
char *strncat(char *destination, const char *source, size_t n)
{
  size_t length = strlen(destination);
  size_t i;

  for (i = 0; i < n && source[i] != '\0'; i++)
  {
    destination[length + i] = source[i];
  }

  destination[length + i] = '\0';
  return destination;
}

// Copy first n characters of src to destination
char *strncpy(char *dest, const char *src, size_t n)
{
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
  {
    dest[i] = src[i];
  }

  for (; i < n; i++)
  {
    dest[i] = '\0';
  }

  return dest;
}

// Copy all of str to dest
char *strcpy(char *dest, const char *src)
{
  return strncpy(dest, src, strlen(src));
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  for (size_t i = 0; i < n && *s1 == *s2; s1++, s2++, i++)
    if (*s1 == '\0')
      return 0;

  return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

int strcmp(const char *s1, const char *s2)
{
  return strncmp(s1, s2, strlen(s1));
}

/*
In place string reverse
*/
char *strrev(char *s)
{
  int length = strlen(s) - 1;

  for (int i = 0; i <= length / 2; i++)
  {
    char temp = s[i];
    s[i] = s[length - i];
    s[length - i] = temp;
  }

  return s;
}

void *memcpy(void *s1, const void *s2, size_t n)
{
  uint8_t *src = (uint8_t *)s2;
  uint8_t *dest = (uint8_t *)s1;

  for (size_t i = 0; i < n; i++)
    dest[i] = src[i];
  return s1;
}

void *memset(void *s, int c, size_t n)
{
  uint8_t *dest = (uint8_t *)s;
  for (size_t i = 0; i < n; i++)
    dest[i] = c;
  return s;
}

char *itoa(uint32_t num, int base)
{
  int i = 0;
  int len = 8;

  char *buffer = malloc(32);
  if (base == 2)
    len = 32;

  if (num == 0 && base == 2)
  {
    while (i < len)
      buffer[i++] = '0';
    buffer[i] = '\0';
    return buffer;
  }

  // go in reverse order
  while (num != 0 && len--)
  {
    int remainder = num % base;
    // case for hexadecimal
    buffer[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
    num = num / base;
  }

  while (len-- && base != 10)
    buffer[i++] = '0';

  buffer[i] = '\0';

  return strrev(buffer);
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

extern uint32_t HEAP;

void *malloc(size_t n)
{
  return (void *)(HEAP += n);
}

void free(void *mem)
{
  UNUSED(mem);
}

int isascii(int c)
{
  return c >= 0 && c <= 127;
}

int isdigit(char c)
{
  return c >= '0' && c <= '9';
}

int islower(char c)
{
  return c >= 'a' && c <= 'z';
}

int isupper(char c)
{
  return c >= 'A' && c <= 'Z';
}

int tolower(char c)
{
  return isdigit(c) ? c : islower(c) ? c : (c - 'A') + 'a';
}

int toupper(char c)
{
  return (isdigit(c) ? c : (isupper(c) ? c : ((c - 'a') + 'A')));
}

char *strchr(const char *s, int c)
{
  while (*s != '\0')
    if (*s++ == c)
      return (char *)s;
  return NULL;
}

char *strtok(char *s, const char *delim)
{
  char *b = NULL;
  static char *ptr;

  if (s)
  {
    ptr = s;
  }

  if (!ptr && !s)
  {
    return NULL;
  }

  for (size_t i = 0; i < strlen(delim); i++)
  {
    b = ptr;
    ptr = strchr(ptr, delim[i]);

    if (!ptr)
    {
      return b;
    }

    *--ptr = '\0';
    ptr++;

    return b;
  }

  return NULL;
}
