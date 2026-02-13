#include "drivers.h"
#include "types.h"

/* Port I/O helper functions */
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

/* PS/2 Keyboard driver
 * Reads scan codes from PS/2 controller port 0x60
 * Uses IRQ 1
 */

#define KB_PORT 0x60
#define KB_STATUS_PORT 0x64
#define KB_STATUS_OUT_FULL 0x01

/* US keyboard scancode map */
static const char scancode_map[] = {
	0, 27,              /* ESC */
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b',               /* Backspace */
	'\t',               /* Tab */
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'\n',               /* Enter */
	0,                  /* Ctrl */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0,                  /* LShift */
	'\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0,                  /* RShift */
	'*',                /* Keypad */
	0,                  /* Alt */
	' ',                /* Space bar */
	0,                  /* Caps lock */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* F1-F10 */
	0,                  /* Num lock */
	0,                  /* Scroll lock */
};

/* Initialize keyboard */
void keyboard_init(void) {
	/* Keyboard is already initialized by BIOS */
	/* Enable PS/2 controller if needed */
	vga_write_string("Keyboard driver loaded\n");
}

/* Read character from keyboard (blocking) */
char keyboard_read_char(void) {
	while (1) {
		/* Check if data is available from keyboard */
		uint8_t status = inb(KB_PORT + 4);  /* 0x60 + 4 = 0x64 */

		if (status & KB_STATUS_OUT_FULL) {
			/* Read scancode */
			uint8_t scancode = inb(KB_PORT);

			/* Check if it's a make code (not break code) */
			if (!(scancode & 0x80)) {
				if (scancode < sizeof(scancode_map)) {
					return scancode_map[scancode];
				}
			}
		}
	}
}
