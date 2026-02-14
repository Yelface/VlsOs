#ifndef UI_H
#define UI_H

#include "types.h"

/* UI color scheme */
#define UI_COLOR_HEADER   (VGA_COLOR_LIGHT_GREY << 4) | VGA_COLOR_BLACK
#define UI_COLOR_FOOTER   (VGA_COLOR_LIGHT_GREY << 4) | VGA_COLOR_BLACK
#define UI_COLOR_NORMAL   VGA_COLOR_LIGHT_GREY
#define UI_COLOR_INPUT    VGA_COLOR_WHITE

/* UI dimensions */
#define UI_HEADER_HEIGHT  1
#define UI_FOOTER_HEIGHT  1
#define UI_CONTENT_ROWS   (25 - UI_HEADER_HEIGHT - UI_FOOTER_HEIGHT)

/* UI state */
typedef struct {
	int enabled;
	int show_cursor;
	uint32_t uptime_ticks;
} ui_state_t;

/* UI functions */
void ui_init(void);
void ui_enable(void);
void ui_disable(void);
void ui_draw_header(void);
void ui_draw_footer(const char* prompt);
void ui_draw_content(const char** lines, int line_count);
void ui_clear_content(void);
void ui_update_header(void);
void ui_print_content(const char* text);
void ui_scroll_content(void);

extern ui_state_t g_ui_state;

#endif
