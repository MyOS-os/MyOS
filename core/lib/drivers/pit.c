#include <kernel/drivers/pit.h>
#include <kernel/drivers/io.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43
#define PIT_INPUT_HZ 1193182

static u32 pit_frequency = 0;
static volatile u64 pit_ticks = 0;

static void pit_set_reload(u16 reload) {
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (u8)(reload & 0xFF));
    outb(PIT_CHANNEL0, (u8)((reload >> 8) & 0xFF));
}

void pit_init(u32 frequency_hz) {
    if (frequency_hz == 0) {
        frequency_hz = 100;
    }
    pit_frequency = frequency_hz;
    u32 reload = PIT_INPUT_HZ / frequency_hz;
    if (reload == 0) {
        reload = 1;
    }
    pit_set_reload((u16)reload);
}

u32 pit_get_frequency(void) {
    return pit_frequency;
}

u64 pit_get_ticks(void) {
    return pit_ticks;
}

void pit_on_tick(void) {
    pit_ticks++;
}
