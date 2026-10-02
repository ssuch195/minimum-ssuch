#ifndef UART_H
#define UART_H

void uart_putc(char c);
void uart_puts(const char *s);
void kprintf(const char *fmt, ...);
void uart_rx_init(void);
char uart_getc(void);

#endif