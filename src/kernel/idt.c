#include "drivers.h"
#include "types.h"

/* Port I/O helper functions */
extern void outb(uint16_t port, uint8_t value);

/* Interrupt handlers */
extern void pit_irq0_handler(void);

/* Interrupt Descriptor Table (IDT) setup
 * x86 supports 256 interrupts (0-255)
 * 0-31: CPU exceptions & faults
 * 32-47: Hardware IRQs (via PIC)
 * 48-255: Software interrupts
 */

#define IDT_ENTRIES 256

/* IDT entry structure */
struct idt_entry {
	uint16_t base_lo;     /* Low 16 bits of handler address */
	uint16_t selector;    /* Kernel code segment selector */
	uint8_t  zero;        /* Always 0 */
	uint8_t  flags;       /* Present, DPL, reserved, gate type */
	uint16_t base_hi;     /* High 16 bits of handler address */
} __attribute__((packed));

/* IDT descriptor */
struct idt_ptr {
	uint16_t limit;       /* Size of IDT - 1 */
	uint32_t base;        /* Base address of IDT */
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

/* Stub handlers - will implement proper ones later */
extern void isr0();   /* Divide by zero */
extern void isr1();   /* Debug exception */
extern void isr8();   /* Double fault */
extern void isr14();  /* Page fault */
extern void irq0();   /* Timer */
extern void irq1();   /* Keyboard */

/* Set an IDT entry */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	idt[num].base_lo = base & 0xFFFF;
	idt[num].base_hi = (base >> 16) & 0xFFFF;
	idt[num].selector = sel;
	idt[num].zero = 0;
	idt[num].flags = flags;
}

/* Initialize IDT */
void idt_init(void) {
	idt_descriptor.base = (uint32_t) &idt;
	idt_descriptor.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;

	/* Clear IDT */
	for (int i = 0; i < IDT_ENTRIES; i++) {
		idt[i].base_lo = 0;
		idt[i].base_hi = 0;
		idt[i].selector = 0;
		idt[i].zero = 0;
		idt[i].flags = 0;
	}

	/* Set up exception handlers */
	idt_set_gate(0, (uint32_t) isr0, 0x08, 0x8E);   /* Divide by zero */
	idt_set_gate(1, (uint32_t) isr1, 0x08, 0x8E);   /* Debug */
	idt_set_gate(8, (uint32_t) isr8, 0x08, 0x8E);   /* Double fault */
	idt_set_gate(14, (uint32_t) isr14, 0x08, 0x8E); /* Page fault */

	/* Set up hardware IRQ handlers */
	idt_set_gate(32, (uint32_t) irq0, 0x08, 0x8E);  /* Timer */
	idt_set_gate(33, (uint32_t) irq1, 0x08, 0x8E);  /* Keyboard */

	/* Load IDT */
	__asm__ volatile("lidt %0" : : "m" (idt_descriptor));
}

/* Exception handlers - stubs for now */
void isr_handler(uint32_t isrnum) {
	vga_write_string("Exception: ");

	switch (isrnum) {
		case 0: vga_write_string("Divide by zero\n"); break;
		case 1: vga_write_string("Debug exception\n"); break;
		case 8: vga_write_string("Double fault\n"); break;
		case 14: vga_write_string("Page fault\n"); break;
		default: vga_write_string("Unknown exception\n");
	}
}

/* IRQ handlers - stubs for now */
void irq_handler(uint32_t irqnum) {
	switch (irqnum) {
		case 0: pit_irq0_handler(); break;
		case 1: break; /* Keyboard IRQ */
		default: break;
	}
	/* Send EOI (End Of Interrupt) to PIC */
	outb(0x20, 0x20);
}
