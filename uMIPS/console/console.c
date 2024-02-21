#pragma GCC optimize ("Ofast")

#include "pico/stdlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "console.h"

#include "../config/umips_config.h"
#include "../emulator/dz11/dz11.h"
#include <string.h>

#include "bsp/rp2040/board.h"
#include "tusb.h"
#include "tusb_config.h"

queue_t ser_screen_queue;
queue_t ser_screen_queuel1;
queue_t ser_screen_queuel2;
queue_t ser_screen_queuel3;
queue_t ser_screen_queuel4;

void console_init(void)
{
    board_init();
    tud_init(BOARD_TUD_RHPORT);
    
    uart_init(UART_INSTANCE, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    queue_init(&ser_screen_queue, sizeof(char), IO_QUEUE_LEN);
    queue_init(&ser_screen_queuel1, sizeof(char), IO_QUEUE_LEN);
    queue_init(&ser_screen_queuel2, sizeof(char), IO_QUEUE_LEN);
    queue_init(&ser_screen_queuel3, sizeof(char), IO_QUEUE_LEN);
    queue_init(&ser_screen_queuel4, sizeof(char), IO_QUEUE_LEN);
}

void ser_console_task(void)
{
    if (!queue_is_empty(&ser_screen_queue))
    {
        uint8_t c;
        queue_remove_blocking(&ser_screen_queue, &c);

        if(c == '\n') uart_putc_raw(UART_INSTANCE, '\r');
        uart_putc_raw(UART_INSTANCE, c);
    }

    uint8_t c;

    if (!queue_is_empty(&ser_screen_queuel1))
    {
        if (tud_cdc_n_connected(0)) {
            queue_remove_blocking(&ser_screen_queuel1, &c);
            if(c == '\n') tud_cdc_write_char('\r');
            tud_cdc_n_write_char(0, c);
        }
    }
    if (!queue_is_empty(&ser_screen_queuel2))
    {
        if (tud_cdc_n_connected(1)) {
            queue_remove_blocking(&ser_screen_queuel2, &c);
            if(c == '\n') tud_cdc_write_char('\r');
            tud_cdc_n_write_char(1, c);
        }
    }
    if (!queue_is_empty(&ser_screen_queuel3))
    {
        if (tud_cdc_n_connected(2)) {
            queue_remove_blocking(&ser_screen_queuel3, &c);
            if(c == '\n') tud_cdc_write_char('\r');
            tud_cdc_n_write_char(2, c);
        }
    }
    if (!queue_is_empty(&ser_screen_queuel4))
    {
        if (tud_cdc_n_connected(3)) {
            queue_remove_blocking(&ser_screen_queuel4, &c);
            if(c == '\n') tud_cdc_write_char('\r');
            tud_cdc_n_write_char(3, c);
        }
    }

    tud_cdc_n_write_flush(0);
    tud_cdc_n_write_flush(1);
    tud_cdc_n_write_flush(2);
    tud_cdc_n_write_flush(3);

    uint8_t uart_in_ch;
    if (uart_is_readable(UART_INSTANCE))
    {
        uart_read_blocking(UART_INSTANCE, &uart_in_ch, 1);
        dz11charRx(3, (uint8_t)uart_in_ch);
    }

    uint8_t buf[IO_QUEUE_LEN];
    if(tud_cdc_n_available(0)) {
        uint32_t count = tud_cdc_n_read(0, buf, sizeof(buf));
        for(int i = 0; i < count; i++)
            dz11charRx(1, (uint8_t)buf[i]);
    }
    if(tud_cdc_n_available(1)) {
        uint32_t count = tud_cdc_n_read(1, buf, sizeof(buf));
        for(int i = 0; i < count; i++)
            dz11charRx(2, (uint8_t)buf[i]);
    }
    if(tud_cdc_n_available(2)) {
        uint32_t count = tud_cdc_n_read(2, buf, sizeof(buf));
        for(int i = 0; i < count; i++)
            dz11charRx(3, (uint8_t)buf[i]);
    }
    if(tud_cdc_n_available(3)) {
        uint32_t count = tud_cdc_n_read(3, buf, sizeof(buf));
        for(int i = 0; i < count; i++)
            dz11charRx(4, (uint8_t)buf[i]);
    }
}

void console_task(void)
{
    tud_task();
    ser_console_task();
}

void console_putc_uart(char c)
{
    queue_try_add(&ser_screen_queue, &c);
    // queue_add_blocking(&ser_screen_queuel1, &c);
}

void console_puts_uart(char s[])
{
    uint8_t n = strlen(s);
    for (int i = 0; i < n; i++)
        console_putc_uart(s[i]);
}

char termPrintBuf[100];

void console_printf_uart(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsprintf(termPrintBuf, format, args);
    console_puts_uart(termPrintBuf);
    va_end(args);
}

void console_panic_uart(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsprintf(termPrintBuf, format, args);
    console_puts_uart("\x1b[31mPANIC: ");
    console_puts_uart(termPrintBuf);
    va_end(args);

    while (true)
        tight_loop_contents();
}

void console_putc_cdc(uint8_t line, char c)
{
    switch(line) {
    default:
    case 3:
        queue_try_add(&ser_screen_queuel3, &c);
        break;
    case 1:
        queue_try_add(&ser_screen_queuel1, &c);
        break;
    case 2:
        queue_try_add(&ser_screen_queuel2, &c);
        break;
    case 4:
        queue_try_add(&ser_screen_queuel4, &c);
        break;
    }
}