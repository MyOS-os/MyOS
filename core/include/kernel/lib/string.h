#pragma once

#include <kernel/lib/types.h>

void *memset(void *s, int c, u64 n);
void *memcpy(void *dest, const void *src, u64 n);
void *memmove(void *dest, const void *src, u64 n);
int memcmp(const void *a, const void *b, u64 n);
void *memchr(const void *s, int c, u64 n);

u64 strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, u64 n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, u64 n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, u64 n);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
u64 strspn(const char *s, const char *accept);
u64 strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strstr(const char *haystack, const char *needle);
