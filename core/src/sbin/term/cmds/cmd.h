#pragma once

#include <kernel/lib/types.h>

typedef struct {
    const char *name;
    const char *desc;
    void (*handler)(int argc, char **argv);
} term_command_t;

const term_command_t *term_commands(void);
u64 term_command_count(void);
void term_register_builtin_commands(void);
