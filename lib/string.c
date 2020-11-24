#include <stdint.h>
#include <stdbool.h>
#include "string.h"

char *itoa(int num, char *str, int base)
{
  int i = 0;
  bool isNegative = false;

  /* Handle 0 explicitely, otherwise empty string is printed for 0 */
  if (num == 0)
  {
    str[i++] = '0';
    str[i] = '\0';
    return str;
  }

  // In standard itoa(), negative numbers are handled only with
  // base 10. Otherwise numbers are considered unsigned.
  if (num < 0 && base == 10)
  {
    isNegative = true;
    num = -num;
  }

  // Process individual digits
  while (num != 0)
  {
    int rem = num % base;
    str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
    num = num / base;
  }

  // If number is negative, append '-'
  if (isNegative)
    str[i++] = '-';

  str[i] = '\0'; // Append string terminator

  // Reverse the string
  strrev(str);

  return str;
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
  int i = 0;
  while (str[i] != '\0')
    ++i;

  return i;
}

// Returns<0 if s1<s2, 0 if s1 == s2, > 0 if s1> s2
int strcmp(const char *s1, const char *s2)
{
  int i;
  for (i = 0; s1[i] == s2[i]; i++)
  {
    if (s1[i] == '\0')
      return 0;
  }
  return s1[i] - s2[i];
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
      continue;

    zeros = 1;

    if (tmp > 0xA)
      append(str, tmp - 0xA + 'a');
    else
      append(str, tmp + '0');
  }

  tmp = n & 0xF;

  if (tmp >= 0xA)
    append(str, tmp - 0xA + 'a');
  else
    append(str, tmp + '0');
}
