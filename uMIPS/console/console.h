#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "pico/util/queue.h"

// #define console_printf(e, ...) console_printf_uart(e)
// #define console_panic(e, ...) console_panic_uart(e)

#define IO_QUEUE_LEN 31

extern queue_t ser_screen_queue, kb_queue;

void console_init(void);
void console_task(void);

void console_putc(char c);
void console_puts(char s[]);

#endif