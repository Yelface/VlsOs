#ifndef DRIVERS_H
#define DRIVERS_H

#include "types.h"

/* VGA Driver */
void vga_init(void);
void vga_write_char(char);
void vga_clear_screen(void);
void vga_set_color(uint8_t foreground, uint8_t background);
void vga_write_string(const char* str);
void vga_set_position(int row, int col);
int vga_get_cursor_x(void);
int vga_get_cursor_y(void);

/* PS/2 Keyboard Driver */
void keyboard_init(void);
char keyboard_read_char(void);

/* Programmable Interval Timer (PIT) */
void pit_init(uint32_t frequency);
uint32_t pit_get_ticks(void);
void pit_wait_ms(uint32_t ms);

/* VGA color constants */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

#endif
