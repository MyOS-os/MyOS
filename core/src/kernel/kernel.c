#include <kernel/drivers/keyboard.h>
#include <kernel/drivers/pit.h>
#include <kernel/drivers/serial.h>
#include <kernel/drivers/vga.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/livecds.h>
#include <kernel/lib/memory.h>
#include <kernel/lib/string.h>
#include <kernel/lib/panic.h>
#include <kernel/sbin/term/term.h>

void kernel_main(void) {
    memory_init(NULL, 0);
    vga_init();
    serial_init(SERIAL_PORT_COM1);
    pit_init(100);
    keyboard_init();
    livecds_init();

    printk_set_output(NULL);

    term_init();
    term_run();

    panic("Kernel terminated unexpectedly");
}
