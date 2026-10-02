#include <stddef.h>
#include "../drivers/uart.h"
#include <minemu/ksys.h>

#define LINE_MAX_LEN 20

static void msh_exec(const char *p) {
    while (*p == ' ') {
        p++;
    }                 
    if (!*p) {
        return;
    }                    

    const char *w = p;                   
    while (*p && *p != ' ') {
        p++;
    }
    size_t wl = (size_t)(p - w);      

    if (wl == 4 && w[0]=='e' && w[1]=='c' && w[2]=='h' && w[3]=='o') {
        while (*p == ' ') {
            p++;
        }           
        uart_puts(p);
        uart_putc('\n');
    } 
    else {
        uart_puts("command not found: ");
        for (size_t i = 0; i < wl; i++) {
            uart_putc(w[i]);
        }
        uart_putc('\n');
    }
}

void msh_run(void) {
    char line[LINE_MAX_LEN + 1];
    size_t len = 0, extra = 0;

    uart_puts("msh> ");
    for (;;) {
        char c = uart_getc();
        if (c == '\n') {
            line[len] = '\0';
            msh_exec(line);
            len = 0;
            extra = 0;
            uart_puts("msh> ");
        } 
        else if (c == 0x08 || c == 0x7f) {
            if (extra) {
                extra--;
            }            
            else if (len) {
                len--;
            }         
        } 
        else if (len < LINE_MAX_LEN) {
            line[len++] = c;
        } 
        else {
            extra++;                      
        }
    }
}