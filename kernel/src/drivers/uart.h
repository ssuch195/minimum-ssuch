#ifndef MINEMU_UART_H
#define MINEMU_UART_H

void uart_putc(char c);
void uart_puts(const char *s);
void kprintf(const char *fmt, ...);

#endif