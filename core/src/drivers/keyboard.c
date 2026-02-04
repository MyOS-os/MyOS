#include <kernel/drivers/keyboard.h>
#include <kernel/drivers/io.h>

#define KBD_DATA 0x60
#define KBD_STATUS 0x64

static const char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7',
    '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void keyboard_init(void) {
    (void)scancode_map;
}

static bool kbd_has_data(void) {
    return (inb(KBD_STATUS) & 0x01) != 0;
}

bool keyboard_has_char(void) {
    return kbd_has_data();
}

char keyboard_read_char(void) {
    while (!kbd_has_data()) {
    }
    u8 scancode = inb(KBD_DATA);
    if (scancode & 0x80) {
        return 0;
    }
    return scancode_map[scancode];
}
