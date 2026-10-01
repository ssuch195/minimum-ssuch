#include <stdarg.h>
#include <stdint.h>
#include "minemu/platform.h"
#include "uart.h"

void uart_putc(char c) {
    while (!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY));
    MINEMU_UART0->tx_data = (uint32_t)(uint8_t) c;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

static void put_uint(uint32_t v, uint32_t base) {
    char buf[32];
    int i = 0;
    if (v == 0) {
        buf[i++] = '0';
    }
    while (v) {
        buf[i++] = "0123456789abcdef"[v % base];
        v /= base;
    }
    while (i) {
        uart_putc(buf[--i]);
    }
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { 
            uart_putc(*fmt); 
            continue; 
        }
        switch (*fmt++) {
        case 's': { const char *s = va_arg(ap, const char *);
                    uart_puts(s ? s : "(null)"); break; }
        case 'c': uart_putc((char)va_arg(ap, int)); break;
        case 'u': put_uint(va_arg(ap, uint32_t), 10); break;
        case 'x': put_uint(va_arg(ap, uint32_t), 16); break;
        case 'd': { int32_t d = va_arg(ap, int32_t);
                    if (d < 0) { uart_putc('-'); put_uint((uint32_t)(-(int64_t)d), 10); }
                    else put_uint((uint32_t)d, 10);
                    break; }
        case '%': uart_putc('%'); break;
        default:  uart_putc('%'); uart_putc(*fmt); break;
        }
    }
    va_end(ap);
}