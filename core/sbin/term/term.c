#include <kernel/drivers/keyboard.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include "cmds/cmd.h"
#include <kernel/sbin/term.h>

#define TERM_INPUT_MAX 128
#define TERM_ARG_MAX 16

static char input_buffer[TERM_INPUT_MAX];
static u64 input_len = 0;

static void term_putchar(char c) {
    vga_write_char(c);
    serial_write_char(SERIAL_PORT_COM1, c);
}

void term_init(void) {
    vga_init();
    serial_init(SERIAL_PORT_COM1);
    printk_set_output(term_putchar);
    term_register_builtin_commands();
    printk("MyOS terminal ready. Type 'help'.\n");
}

void term_print_prompt(void) {
    printk("> ");
}

static void reset_input(void) {
    kmemset(input_buffer, 0, sizeof(input_buffer));
    input_len = 0;
}

static int split_args(char *line, char **argv, int max_args) {
    int argc = 0;
    while (*line && argc < max_args) {
        while (*line == ' ') {
            line++;
        }
        if (!*line) {
            break;
        }
        argv[argc++] = line;
        while (*line && *line != ' ') {
            line++;
        }
        if (*line) {
            *line++ = '\0';
        }
    }
    return argc;
}

static void execute_command(char *line) {
    char *argv[TERM_ARG_MAX] = {0};
    int argc = split_args(line, argv, TERM_ARG_MAX);
    if (argc == 0) {
        return;
    }
    const term_command_t *cmds = term_commands();
    u64 count = term_command_count();
    for (u64 i = 0; i < count; i++) {
        if (strcmp(argv[0], cmds[i].name) == 0) {
            cmds[i].handler(argc, argv);
            return;
        }
    }
    printk("Unknown command: %s\n", argv[0]);
}

static void handle_backspace(void) {
    if (input_len == 0) {
        return;
    }
    input_len--;
    input_buffer[input_len] = '\0';
    term_putchar('\b');
    term_putchar(' ');
    term_putchar('\b');
}

void term_run(void) {
    reset_input();
    term_print_prompt();
    while (1) {
        char c = keyboard_read_char();
        if (c == 0) {
            continue;
        }
        if (c == '\n') {
            term_putchar('\n');
            execute_command(input_buffer);
            reset_input();
            term_print_prompt();
            continue;
        }
        if (c == '\b') {
            handle_backspace();
            continue;
        }
        if (input_len + 1 >= TERM_INPUT_MAX) {
            continue;
        }
        input_buffer[input_len++] = c;
        input_buffer[input_len] = '\0';
        term_putchar(c);
    }
}
