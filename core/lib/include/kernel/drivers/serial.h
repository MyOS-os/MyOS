#pragma once

#include <kernel/lib/types.h>

#define SERIAL_PORT_COM1 0x3F8

void serial_init(u16 port);
bool serial_can_read(u16 port);
bool serial_can_write(u16 port);
void serial_write_char(u16 port, char c);
char serial_read_char(u16 port);
void serial_write(u16 port, const char *s);
