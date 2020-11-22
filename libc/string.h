#ifndef STRING_H
#define STRING_H

char *itoa(int num, char *str, int base);
void strrev(char s[]);
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
void backspace(char *s);
void append(char *s, char n);
void hex_to_ascii(int n, char *str);

#endif
