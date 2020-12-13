#pragma once

#include <stdint.h>
#include <stddef.h>
#include "boot/stage2/vga.h"

#define assert(e) ((e) ? (void)0 : vga_pretty_assert(#e, VGA_RED))

size_t strlen(const char *s);
char *strncat(char *destination, const char *source, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strcpy(char *dest, const char *src);
int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
char *strrev(char *s);
char *strchr(const char *s, int c);
char *strtok(char *s, const char *delim);

int isascii(char c);
int isdigit(char c);
int islower(char c);
int isupper(char c);
int tolower(char c);
int toupper(char c);

char *itoa(uint32_t num, int base);

void *memcpy(void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
void *malloc(size_t n);
void free(void *mem);

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint16_t data);
void insl(int port, void *addr, int cnt);
