#include "drivers.h"
#include "types.h"

/* Port I/O helper functions */
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

/* Programmable Interval Timer (PIT) driver
 * Uses Intel 8253/8254 PIT chip
 * IRQ 0, I/O ports 0x40-0x43
 */

#define PIT_PORT_0    0x40
#define PIT_PORT_1    0x41
#define PIT_PORT_2    0x42
#define PIT_CONTROL   0x43

#define PIT_FREQUENCY 1193182  /* Base frequency of PIT in Hz */

static uint32_t ticks = 0;

/* PIT interrupt handler */
void pit_irq0_handler(void) {
	ticks++;
	/* Send EOI to PIC */
	outb(0x20, 0x20);
}

/* Initialize PIT to generate interrupts at specified frequency */
void pit_init(uint32_t frequency) {
	if (frequency == 0) frequency = 1;

	uint32_t divisor = PIT_FREQUENCY / frequency;

	/* Select channel 0, 16-bit count, square wave (mode 3) */
	uint8_t cmd = 0x34;  /* 00 (ch0) 11 (lo/hi) 010 (mode 2) 0 (binary) */
	outb(PIT_CONTROL, cmd);

	/* Set divisor */
	uint8_t lo = divisor & 0xFF;
	uint8_t hi = (divisor >> 8) & 0xFF;

	outb(PIT_PORT_0, lo);
	outb(PIT_PORT_0, hi);

	ticks = 0;
}

/* Get number of ticks since initialization */
uint32_t pit_get_ticks(void) {
	return ticks;
}

/* Wait for specified number of milliseconds */
void pit_wait_ms(uint32_t ms) {
	uint32_t start = ticks;
	uint32_t wait_ticks = (ms * 1000) / PIT_FREQUENCY;

	while ((ticks - start) < wait_ticks) {
		__asm__ volatile("hlt");
	}
}
