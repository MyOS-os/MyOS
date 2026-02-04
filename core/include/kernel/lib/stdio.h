#pragma once

#include <kernel/lib/types.h>

typedef void (*putchar_fn)(char);

void printk_set_output(putchar_fn output);
const char *printk_buffer(void);
u64 printk_buffer_len(void);

int printk(const char *fmt, ...);
