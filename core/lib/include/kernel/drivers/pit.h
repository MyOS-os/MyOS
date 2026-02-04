#pragma once

#include <kernel/lib/types.h>

void pit_init(u32 frequency_hz);
u32 pit_get_frequency(void);
u64 pit_get_ticks(void);
void pit_on_tick(void);
