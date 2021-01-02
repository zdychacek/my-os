#include "lib/types.h"
#include "lib/string.h"

char *itoa(unsigned int num, char *str, int base)
{
  int i = 0;
  // bool isNegative = false;

  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num == 0)
  {
    str[i++] = '0';
    str[i] = '\0';

    return str;
  }

  // TODO: implement support for negative numbers
  // In standard itoa(), negative numbers are handled only with
  // base 10. Otherwise numbers are considered unsigned.
  // if (num < 0 && base == 10)
  // {
  //   isNegative = true;
  //   num = -num;
  // }

  // Process individual digits
  while (num != 0)
  {
    int rem = num % base;

    str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
    num = num / base;
  }

  // If number is negative, append '-'
  // if (isNegative)
  //   str[i++] = '-';

  str[i] = '\0'; // Append string terminator

  // Reverse the string
  strrev(str);

  return str;
}

// In place string reverse
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

size_t strlen(const char *str)
{
  int i = 0;

  while (str[i] != '\0')
  {
    ++i;
  }

  return i;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
  for (size_t i = 0; i < n && *s1 == *s2; s1++, s2++, i++)
  {
    if (*s1 == '\0')
    {
      return 0;
    }
  }

  return (*(unsigned char *)s1 - *(unsigned char *)s2);
}

int strcmp(const char *s1, const char *s2)
{
  return strncmp(s1, s2, strlen(s1));
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

char *strchr(const char *s, int c)
{
  while (*s != '\0')
  {
    if (*s++ == c)
    {
      return (char *)s;
    }
  }

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

void append(char s[], char n)
{
  int len = strlen(s);

  s[len] = n;
  s[len + 1] = '\0';
}

void backspace(char s[])
{
  int len = strlen(s);

  s[len - 1] = '\0';
}

void hex_to_ascii(int n, char *str)
{
  append(str, '0');
  append(str, 'x');
  char zeros = 0;

  int tmp;

  for (int i = 28; i > 0; i -= 4)
  {
    tmp = (n >> i) & 0xF;

    if (tmp == 0 && zeros == 0)
    {
      continue;
    }

    zeros = 1;

    if (tmp > 0xA)
    {
      append(str, tmp - 0xA + 'a');
    }
    else
    {
      append(str, tmp + '0');
    }
  }

  tmp = n & 0xF;

  if (tmp >= 0xA)
  {
    append(str, tmp - 0xA + 'a');
  }
  else
  {
    append(str, tmp + '0');
  }
}

bool isascii(int c)
{
  return c >= 0 && c <= 127;
}

bool isdigit(char c)
{
  return c >= '0' && c <= '9';
}

bool islower(char c)
{
  return c >= 'a' && c <= 'z';
}

bool isupper(char c)
{
  return c >= 'A' && c <= 'Z';
}

bool tolower(char c)
{
  return isdigit(c) ? c : islower(c) ? c : (c - 'A') + 'a';
}

bool toupper(char c)
{
  return (isdigit(c) ? c : (isupper(c) ? c : ((c - 'a') + 'A')));
}

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
  {
    *temp++ = (uint8_t)val;
  }
}
