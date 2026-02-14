#include "ui.h"
#include "drivers.h"
#include "string.h"
#include "memory.h"
#include "net.h"

ui_state_t g_ui_state = {0};

/* Content buffer for UI display */
static char content_buffer[UI_CONTENT_ROWS * 80];
static int content_lines = 0;
static int content_scroll = 0;

void ui_init(void) {
	memset(&g_ui_state, 0, sizeof(ui_state_t));
	memset(content_buffer, 0, sizeof(content_buffer));
	g_ui_state.uptime_ticks = 0;
}

void ui_enable(void) {
	g_ui_state.enabled = 1;
	vga_clear_screen();
	ui_draw_header();
	ui_draw_footer("vlsos> ");
}

void ui_disable(void) {
	g_ui_state.enabled = 0;
	vga_clear_screen();
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ui_draw_header(void) {
	/* Position cursor at top-left */
	vga_set_position(0, 0);
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
	
	/* Draw header line */
	vga_write_string("VlsOs OS");
	
	/* Padding to right edge */
	for (int i = 8; i < 80; i++) {
		vga_write_char(' ');
	}
	
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ui_draw_footer(const char* prompt) {
	/* Position cursor at bottom row (24) */
	vga_set_position(0, 24);
	vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
	
	/* Draw prompt */
	if (prompt) {
		vga_write_string(prompt);
	}
	
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void ui_clear_content(void) {
	/* Clear rows 1-23 */
	for (int row = 1; row < 24; row++) {
		vga_set_position(0, row);
		for (int col = 0; col < 80; col++) {
			vga_write_char(' ');
		}
	}
	vga_set_position(0, 1);
	content_lines = 0;
	content_scroll = 0;
}

void ui_scroll_content(void) {
	/* Shift content up one line */
	if (content_lines < UI_CONTENT_ROWS) return;
	
	/* Move all lines up by one in buffer */
	for (int i = 0; i < (UI_CONTENT_ROWS - 1) * 80; i++) {
		content_buffer[i] = content_buffer[i + 80];
	}
	/* Clear last line */
	for (int i = (UI_CONTENT_ROWS - 1) * 80; i < UI_CONTENT_ROWS * 80; i++) {
		content_buffer[i] = 0;
	}
	
	/* Redraw all lines */
	for (int row = 1; row < 24; row++) {
		vga_set_position(0, row);
		for (int col = 0; col < 80; col++) {
			int idx = (row - 1) * 80 + col;
			char c = content_buffer[idx];
			vga_write_char(c ? c : ' ');
		}
	}
}

void ui_print_content(const char* text) {
	if (!text) return;
	
	/* Find current cursor position */
	int row = 1 + content_lines;
	if (row >= 24) {
		ui_scroll_content();
		row = 23;
	}
	
	vga_set_position(0, row);
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	/* Print text and track in buffer */
	int col = 0;
	for (int i = 0; text[i] && col < 80; i++) {
		vga_write_char(text[i]);
		content_buffer[content_lines * 80 + col] = text[i];
		col++;
	}
	
	content_lines++;
	if (content_lines >= UI_CONTENT_ROWS) {
		content_lines = UI_CONTENT_ROWS - 1;
	}
}

void ui_draw_content(const char** lines, int line_count) {
	ui_clear_content();
	for (int i = 0; i < line_count && i < UI_CONTENT_ROWS; i++) {
		vga_set_position(0, 1 + i);
		vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		if (lines[i]) {
			vga_write_string(lines[i]);
		}
	}
	content_lines = line_count;
}

void ui_update_header(void) {
	/* Update header with current status */
	vga_set_position(0, 0);
	vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
	
	/* Draw title */
	vga_write_string("VlsOs");
	
	/* Draw status from right: network up/down, time, etc */
	vga_set_position(60, 0);
	
	/* Show network interface status */
	net_interface_t* iface = net_get_interface("eth0");
	if (iface && (iface->flags & 2)) {
		vga_write_string("[NET UP]");
	} else {
		vga_write_string("[NET DOWN]");
	}
	
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}
