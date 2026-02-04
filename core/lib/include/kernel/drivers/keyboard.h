#pragma once

#include <kernel/lib/types.h>

void keyboard_init(void);
bool keyboard_has_char(void);
char keyboard_read_char(void);
