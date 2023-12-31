#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "pico/util/queue.h"

#define IO_QUEUE_LEN 15

extern queue_t ser_screen_queue;

void console_init(void);
void console_task(void);

void console_putc(char c);
void console_puts(char s[]);
void console_printf(const char *format, ...);
void console_panic(const char *format, ...);

#endif