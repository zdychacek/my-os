#include "./util.h"

void memory_copy(char *source, char *dest, int nbytes)
{
  for (int i = 0; i < nbytes; i++)
  {
    *(dest + i) = *(source + i);
  }
}

/**
 * K&R implementation
 */
void int_to_ascii(int n, char str[])
{
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do
  {
    str[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    str[i++] = '-';
  str[i] = '\0';

  strrev(str);
}

void strrev(char *str)
{
  char a;
  int len = strlen((const char *)str);

  for (int i = 0, j = len - 1; i < j; i++, j--)
  {
    a = str[i];
    str[i] = str[j];
    str[j] = a;
  }
}

int strlen(const char *str)
{
  if (!str)
  {
    return 0;
  }

  const char *ptr = str;

  while (*str)
  {
    ++str;
  }

  return str - ptr;
}
