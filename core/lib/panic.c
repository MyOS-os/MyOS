#include <kernel/lib/panic.h>

void panic(const char *msg) {
    (void)msg;
    for (;;) {
        __asm__ volatile("hlt");
    }
}
