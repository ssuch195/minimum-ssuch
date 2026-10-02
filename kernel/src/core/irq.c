#include <stdint.h>
#include <minemu/platform.h>
#include <minemu/irq.h>
#include <minemu/ksys.h>

#define MAX_IRQ 32
static irq_handler_t irq_table[MAX_IRQ];

void irq_register(unsigned id, irq_handler_t fn) {
    if (id < MAX_IRQ) irq_table[id] = fn;
}

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    int32_t id = frame->exception_id;
    if (id >= 0 && id < MAX_IRQ && irq_table[id]) {
        irq_table[id]();
    }
    MINEMU_INTERRUPT->eoi = (uint32_t) id;
    return frame;
}