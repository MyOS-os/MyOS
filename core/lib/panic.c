#include <kernel/lib/panic.h>
#include <kernel/lib/stdio.h>

__attribute__((noreturn)) void panic(const char *msg) {
    if (msg) {
        printk("[panic] %s\n", msg);
    } else {
        printk("[panic] unknown error\n");
    }

    __asm__ volatile("cli");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
