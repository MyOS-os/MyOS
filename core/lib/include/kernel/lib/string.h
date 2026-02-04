#pragma once

#include <kernel/lib/types.h>

void *memset(void *s, int c, u64 n);
void *memcpy(void *dest, const void *src, u64 n);
u64 strlen(const char *s);
