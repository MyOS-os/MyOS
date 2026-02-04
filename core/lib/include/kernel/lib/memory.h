#pragma once

#include <kernel/lib/types.h>

// Базовый heap-аллокатор ядра.
// Требует вызова memory_init() перед использованием.

void memory_init(void *heap_base, u64 heap_size);

void *kmalloc(u64 size);
void *kcalloc(u64 count, u64 size);
void *krealloc(void *ptr, u64 new_size);
void kfree(void *ptr);

void kmemset(void *ptr, u8 val, u64 size);
void kmemcpy(void *dest, const void *src, u64 size);
