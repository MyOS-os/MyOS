#include <kernel/lib/string.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/types.h>
#include <kernel/drivers/vga.h>
#include <kernel/drivers/pit.h>
#include <kernel/drivers/serial.h>
#include "cmd.h"

static void cmd_help(int argc, char **argv);
static void cmd_clear(int argc, char **argv);
static void cmd_echo(int argc, char **argv);
static void cmd_info(int argc, char **argv);
static void cmd_uptime(int argc, char **argv);

static term_command_t builtin_commands[] = {
    {"help", "Список команд", cmd_help},
    {"clear", "Очистить экран", cmd_clear},
    {"echo", "Вывести текст", cmd_echo},
    {"info", "Информация о системе", cmd_info},
    {"uptime", "Показать тики таймера", cmd_uptime}
};

const term_command_t *term_commands(void) {
    return builtin_commands;
}

u64 term_command_count(void) {
    return sizeof(builtin_commands) / sizeof(builtin_commands[0]);
}

void term_register_builtin_commands(void) {
    (void)builtin_commands;
}

static void cmd_help(int argc, char **argv) {
    (void)argc;
    (void)argv;
    const term_command_t *cmds = term_commands();
    u64 count = term_command_count();
    for (u64 i = 0; i < count; i++) {
        printk("%s - %s\n", cmds[i].name, cmds[i].desc);
    }
}

static void cmd_clear(int argc, char **argv) {
    (void)argc;
    (void)argv;
    vga_clear();
}

static void cmd_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printk("%s", argv[i]);
        if (i + 1 < argc) {
            printk(" ");
        }
    }
    printk("\n");
}

static void cmd_info(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printk("MyOS kernel - basic terminal\n");
    printk("Serial: COM1, VGA text mode\n");
}

static void cmd_uptime(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printk("ticks: %u\n", (u32)pit_get_ticks());
}
