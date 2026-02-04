#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>

#define PRINTK_BUFFER_SIZE 1024

static char printk_ring[PRINTK_BUFFER_SIZE];
static u64 printk_head = 0;
static u64 printk_len = 0;
static putchar_fn printk_output = NULL;

static void printk_write_char(char c) {
    if (printk_output) {
        printk_output(c);
    }

    printk_ring[printk_head] = c;
    printk_head = (printk_head + 1) % PRINTK_BUFFER_SIZE;
    if (printk_len < PRINTK_BUFFER_SIZE) {
        printk_len++;
    }
}

void printk_set_output(putchar_fn output) {
    printk_output = output;
}

const char *printk_buffer(void) {
    return printk_ring;
}

u64 printk_buffer_len(void) {
    return printk_len;
}

static u64 utoa_base(char *out, u64 value, u64 base, bool upper) {
    static const char *digits_lower = "0123456789abcdef";
    static const char *digits_upper = "0123456789ABCDEF";
    const char *digits = upper ? digits_upper : digits_lower;
    char tmp[32];
    u64 idx = 0;

    if (value == 0) {
        out[0] = '0';
        return 1;
    }

    while (value > 0 && idx < sizeof(tmp)) {
        tmp[idx++] = digits[value % base];
        value /= base;
    }

    for (u64 i = 0; i < idx; i++) {
        out[i] = tmp[idx - i - 1];
    }

    return idx;
}

static u64 itoa_base(char *out, s64 value, u64 base) {
    if (value < 0) {
        out[0] = '-';
        return 1 + utoa_base(out + 1, (u64)(-value), base, false);
    }
    return utoa_base(out, (u64)value, base, false);
}

static void printk_write(const char *s, u64 len) {
    for (u64 i = 0; i < len; i++) {
        printk_write_char(s[i]);
    }
}

static int vprintk(const char *fmt, __builtin_va_list args) {
    u64 written = 0;
    for (u64 i = 0; fmt[i]; i++) {
        if (fmt[i] != '%') {
            printk_write_char(fmt[i]);
            written++;
            continue;
        }

        i++;
        if (!fmt[i]) {
            break;
        }

        char buffer[64];
        u64 len = 0;

        switch (fmt[i]) {
            case '%':
                printk_write_char('%');
                written++;
                break;
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                printk_write_char(c);
                written++;
                break;
            }
            case 's': {
                const char *s = __builtin_va_arg(args, const char *);
                if (!s) {
                    s = "(null)";
                }
                len = strlen(s);
                printk_write(s, len);
                written += len;
                break;
            }
            case 'd':
            case 'i': {
                s64 val = __builtin_va_arg(args, s64);
                len = itoa_base(buffer, val, 10);
                printk_write(buffer, len);
                written += len;
                break;
            }
            case 'u': {
                u64 val = __builtin_va_arg(args, u64);
                len = utoa_base(buffer, val, 10, false);
                printk_write(buffer, len);
                written += len;
                break;
            }
            case 'x':
            case 'X': {
                u64 val = __builtin_va_arg(args, u64);
                bool upper = (fmt[i] == 'X');
                len = utoa_base(buffer, val, 16, upper);
                printk_write(buffer, len);
                written += len;
                break;
            }
            case 'p': {
                u64 val = (u64)__builtin_va_arg(args, void *);
                printk_write("0x", 2);
                len = utoa_base(buffer, val, 16, false);
                printk_write(buffer, len);
                written += len + 2;
                break;
            }
            default:
                printk_write_char('%');
                printk_write_char(fmt[i]);
                written += 2;
                break;
        }
    }

    return (int)written;
}

int printk(const char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    int written = vprintk(fmt, args);
    __builtin_va_end(args);
    return written;
}
