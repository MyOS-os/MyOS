#include <kernel/drivers/vga.h>
#include <kernel/drivers/io.h>
#include <kernel/lib/string.h>

static u16 cursor_row = 0;
static u16 cursor_col = 0;
static u8 vga_color = 0;
static u16 *const vga_buffer = (u16 *)0xB8000;

static inline u16 vga_entry(char c, u8 color) {
    return (u16)c | (u16)color << 8;
}

static void vga_update_cursor(void) {
    u16 pos = cursor_row * VGA_WIDTH + cursor_col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_color = (u8)(fg | (bg << 4));
}

void vga_clear(void) {
    for (u16 y = 0; y < VGA_HEIGHT; y++) {
        for (u16 x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_color);
        }
    }
    cursor_row = 0;
    cursor_col = 0;
    vga_update_cursor();
}

void vga_init(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

static void vga_scroll_if_needed(void) {
    if (cursor_row < VGA_HEIGHT) {
        return;
    }
    u16 line_size = VGA_WIDTH * sizeof(u16);
    for (u16 row = 1; row < VGA_HEIGHT; row++) {
        kmemcpy(&vga_buffer[(row - 1) * VGA_WIDTH],
                &vga_buffer[row * VGA_WIDTH],
                line_size);
    }
    for (u16 col = 0; col < VGA_WIDTH; col++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', vga_color);
    }
    cursor_row = VGA_HEIGHT - 1;
}

void vga_write_char(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        vga_scroll_if_needed();
        vga_update_cursor();
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        vga_update_cursor();
        return;
    }
    if (c == '\t') {
        cursor_col = (cursor_col + 4) & ~(u16)3;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
            vga_scroll_if_needed();
        }
        vga_update_cursor();
        return;
    }
    vga_buffer[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, vga_color);
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        cursor_col = 0;
        cursor_row++;
        vga_scroll_if_needed();
    }
    vga_update_cursor();
}

void vga_write(const char *s) {
    if (!s) {
        return;
    }
    while (*s) {
        vga_write_char(*s++);
    }
}

void vga_set_cursor(u16 row, u16 col) {
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH) {
        return;
    }
    cursor_row = row;
    cursor_col = col;
    vga_update_cursor();
}
