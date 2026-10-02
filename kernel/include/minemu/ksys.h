#ifndef MINEMU_KSYS_H
#define MINEMU_KSYS_H

typedef void (*irq_handler_t)(void);
void irq_register(unsigned id, irq_handler_t fn);
void msh_run(void) __attribute__((noreturn));

#endif