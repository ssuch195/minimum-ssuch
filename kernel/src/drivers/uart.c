#include <stdarg.h>
#include <stdint.h>
#include <minemu/platform.h>
#include <minemu/irq.h>
#include <minemu/ksys.h>
#include "uart.h"

void uart_putc(char c) {
    while (!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY))
        ;
    MINEMU_UART0->tx_data = (uint32_t)(uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

static void put_uint(uint32_t v, uint32_t base) {
    char buf[32];
    int i = 0;
    if (v == 0) buf[i++] = '0';
    while (v) {
        buf[i++] = "0123456789abcdef"[v % base];
        v /= base;
    }
    while (i) uart_putc(buf[--i]);
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { uart_putc(*fmt); continue; }
        switch (*++fmt) {
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

#define RXBUF_SIZE 64
static volatile uint8_t rxbuf[RXBUF_SIZE];
static volatile uint32_t rx_head;  
static volatile uint32_t rx_tail;   

static void uart0_rx_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t b = (uint8_t)MINEMU_UART0->rx_data;  
        uint32_t next = (rx_head + 1) % RXBUF_SIZE;
        if (next != rx_tail) {        
            rxbuf[rx_head] = b;
            rx_head = next;
        }
    }
}

void uart_rx_init(void) {
    irq_register(MINEMU_IRQ_UART0, uart0_rx_handler);
    MINEMU_INTERRUPT->enable |= UINT32_C(1) << MINEMU_IRQ_UART0;
    MINEMU_UART0->control |= MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
}

static inline uint32_t irq_save(void) {
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr\n\tcpsid i" : "=r"(cpsr) : : "memory");
    return cpsr;
}

static inline void irq_restore(uint32_t cpsr) {
    if (!(cpsr & 0x80)) {
        minemu_irq_enable();   
    }
}

char uart_getc(void) {
    for (;;) {
        uint32_t flags = irq_save();  
        if (rx_tail != rx_head) {
            char c = (char)rxbuf[rx_tail];
            rx_tail = (rx_tail + 1) % RXBUF_SIZE;
            irq_restore(flags);
            return c;
        }
        irq_restore(flags);            
    }
}