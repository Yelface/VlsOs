#include "drivers.h"
#include "types.h"

/* VGA text mode driver
 * VGA memory @ 0xB8000
 * 80 columns x 25 rows
 * Each character is 2 bytes: character code, color attribute
 */

#define VGA_ADDRESS  0xB8000
#define VGA_WIDTH    80
#define VGA_HEIGHT   25

static volatile uint16_t* vga_buffer = (volatile uint16_t*) VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t color = 0x0F;  /* White on black */

static void vga_update_cursor(void);
static uint16_t vga_entry(unsigned char uc, uint8_t attr);
static void vga_scroll_down(void);

/* Make a VGA entry from character and color */
static uint16_t vga_entry(unsigned char uc, uint8_t attr) {
	return (uint16_t) uc | (uint16_t) attr << 8;
}

/* Port I/O helper functions */
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

/* Update cursor position in VGA hardware */
static void vga_update_cursor(void) {
	uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

	/* Send low byte command */
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));

	/* Send high byte command */
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* Scroll display down one line */
static void vga_scroll_down(void) {
	/* Copy lines up */
	for (int y = 0; y < VGA_HEIGHT - 1; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
		}
	}

	/* Clear bottom line */
	for (int x = 0; x < VGA_WIDTH; x++) {
		vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', color);
	}

	cursor_y = VGA_HEIGHT - 1;
}

/* Initialize VGA driver */
void vga_init(void) {
	cursor_x = 0;
	cursor_y = 0;
	color = 0x0F;
	vga_update_cursor();
}

/* Write a single character */
void vga_write_char(char c) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c == '\t') {
		cursor_x = (cursor_x + 8) & ~7;
	} else if (c == '\b') {
		if (cursor_x > 0) cursor_x--;
		vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', color);
	} else {
		vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, color);
		cursor_x++;
	}

	/* Handle line wrapping */
	if (cursor_x >= VGA_WIDTH) {
		cursor_x = 0;
		cursor_y++;
	}

	/* Handle vertical scrolling */
	if (cursor_y >= VGA_HEIGHT) {
		vga_scroll_down();
	}

	vga_update_cursor();
}

/* Clear screen */
void vga_clear_screen(void) {
	for (int y = 0; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', color);
		}
	}
	cursor_x = 0;
	cursor_y = 0;
	vga_update_cursor();
}

/* Set foreground and background colors */
void vga_set_color(uint8_t foreground, uint8_t background) {
	color = (background << 4) | foreground;
}

/* Write a string */
void vga_write_string(const char* str) {
	while (*str) {
		vga_write_char(*str);
		str++;
	}
}

/* Get cursor X position */
int vga_get_cursor_x(void) {
	return cursor_x;
}

/* Get cursor Y position */
int vga_get_cursor_y(void) {
	return cursor_y;
}

/* Set cursor position */
void vga_set_position(int row, int col) {
	if (row >= 0 && row < VGA_HEIGHT && col >= 0 && col < VGA_WIDTH) {
		cursor_y = row;
		cursor_x = col;
		vga_update_cursor();
	}
}
