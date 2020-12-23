#pragma once

#include "lib/types.h"

char *itoa(int num, char *str, int base);
char *strrev(char *s);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
void backspace(char *s);
void append(char *s, char n);
void hex_to_ascii(int n, char *str);
char *strncat(char *destination, const char *source, size_t n);
char *strchr(const char *s, int c);
char *strtok(char *s, const char *delim);

int isascii(int c);
int isdigit(char c);
int islower(char c);
int isupper(char c);
int tolower(char c);
int toupper(char c);

void memcpy(void *dest, const void *src, size_t len);
void memset(void *dest, int val, size_t n);
