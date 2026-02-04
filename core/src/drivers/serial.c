#include <kernel/drivers/serial.h>
#include <kernel/drivers/io.h>

static bool serial_initialized = false;

static u16 select_port(u16 port) {
    return port ? port : SERIAL_PORT_COM1;
}

void serial_init(u16 port) {
    u16 p = select_port(port);
    outb(p + 1, 0x00);
    outb(p + 3, 0x80);
    outb(p + 0, 0x03);
    outb(p + 1, 0x00);
    outb(p + 3, 0x03);
    outb(p + 2, 0xC7);
    outb(p + 4, 0x0B);
    serial_initialized = true;
}

bool serial_can_read(u16 port) {
    u16 p = select_port(port);
    return (inb(p + 5) & 0x01) != 0;
}

bool serial_can_write(u16 port) {
    u16 p = select_port(port);
    return (inb(p + 5) & 0x20) != 0;
}

void serial_write_char(u16 port, char c) {
    u16 p = select_port(port);
    if (!serial_initialized) {
        serial_init(p);
    }
    while (!serial_can_write(p)) {
    }
    outb(p, (u8)c);
}

char serial_read_char(u16 port) {
    u16 p = select_port(port);
    if (!serial_initialized) {
        serial_init(p);
    }
    while (!serial_can_read(p)) {
    }
    return (char)inb(p);
}

void serial_write(u16 port, const char *s) {
    u16 p = select_port(port);
    if (!s) {
        return;
    }
    while (*s) {
        if (*s == '\n') {
            serial_write_char(p, '\r');
        }
        serial_write_char(p, *s++);
    }
}
