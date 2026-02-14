#include "drivers.h"
#include "string.h"
#include "types.h"
#include "shell.h"
#include "net.h"
#include "search.h"

/* Interactive command shell for VlsOs */

#define BUFFER_SIZE 256
#define MAX_ARGS    16

/* Command structure */
struct command {
	const char* name;
	int (*handler)(int argc, char** argv);
	const char* help;
};

/* Built-in commands */
static int cmd_help(int argc, char** argv);
static int cmd_echo(int argc, char** argv);

/* Network commands (external) */
extern int cmd_ifconfig(int argc, char** argv);
extern int cmd_ping(int argc, char** argv);
extern int cmd_netstat(int argc, char** argv);
extern int cmd_arp(int argc, char** argv);
extern int cmd_route(int argc, char** argv);
extern int cmd_http(int argc, char** argv);
static int cmd_clear(int argc, char** argv);
static int cmd_uptime(int argc, char** argv);
static int cmd_exit(int argc, char** argv);
static int cmd_ls(int argc, char** argv);
static int cmd_search(int argc, char** argv);

/* Command table */
static struct command commands[] = {
	{"help",     cmd_help,      "Display available commands"},
	{"echo",     cmd_echo,      "Echo arguments to display"},
	{"clear",    cmd_clear,     "Clear the screen"},
	{"uptime",   cmd_uptime,    "Show system uptime"},
	{"ls",       cmd_ls,        "List files (simulation)"},
	{"exit",     cmd_exit,      "Exit the shell"},
	{"ifconfig", cmd_ifconfig,  "Show network interface configuration"},
	{"ping",     cmd_ping,      "Send ICMP echo request (ping)"},
	{"netstat",  cmd_netstat,   "Show network connections"},
	{"arp",      cmd_arp,       "Show ARP cache"},
	{"route",    cmd_route,     "Show routing table"},
	{"http",     cmd_http,      "HTTP server control (start|stop|status)"},
	{NULL,       NULL,          NULL}
};

/* Command: help */
static int cmd_help(int argc, char** argv) {
	(void) argc;
	(void) argv;

	vga_write_string("VlsOs Command Reference\n");
	vga_write_string("=======================\n\n");

	for (int i = 0; commands[i].name; i++) {
		vga_write_string(commands[i].name);
		vga_write_string(" - ");
		vga_write_string(commands[i].help);
		vga_write_string("\n");
	}

	return 0;
}

/* Command: echo */
static int cmd_echo(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		vga_write_string(argv[i]);
		if (i < argc - 1) vga_write_string(" ");
	}
	vga_write_string("\n");
	return 0;
}

/* Command: clear */
static int cmd_clear(int argc, char** argv) {
	(void) argc;
	(void) argv;
	vga_clear_screen();
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	return 0;
}

/* Command: uptime */
static int cmd_uptime(int argc, char** argv) {
	(void) argc;
	(void) argv;

	uint32_t ticks = pit_get_ticks();
	vga_write_string("Uptime: ");

	char buffer[16];
	itoa(ticks, buffer, 10);
	vga_write_string(buffer);
	vga_write_string(" ticks\n");

	return 0;
}

/* Command: exit */
static int cmd_exit(int argc, char** argv) {
	(void) argc;
	(void) argv;
	return -1;  /* Signal shell to exit */
}

/* Command: ls */
static int cmd_ls(int argc, char** argv) {
	(void) argc;
	(void) argv;

	vga_write_string("(No filesystem yet - simulation only)\n");
	vga_write_string("boot/\n");
	vga_write_string("kernel/\n");
	vga_write_string("bin/\n");

	return 0;
}

/* Parse command line into argc/argv */
static int parse_command(const char* line, char** argv, int max_args) {
	int argc = 0;
	static char buffer[BUFFER_SIZE];

	strcpy(buffer, line);

	char* token = buffer;
	char* start = buffer;

	while (*token && argc < max_args) {
		if (isspace(*token)) {
			*token = 0;
			if (start < token && *start) {
				argv[argc++] = start;
			}
			start = token + 1;
		}
		token++;
	}

	if (start < token && *start) {
		argv[argc++] = start;
	}

	return argc;
}

/* Main shell loop */
void shell_main(void) {
	char buffer[BUFFER_SIZE];
	char* argv[MAX_ARGS];

	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	while (1) {
		/* Print prompt */
		vga_write_string("vlsos> ");

		/* Read command line */
		int pos = 0;
		while (pos < BUFFER_SIZE - 1) {
			char c = keyboard_read_char();

			if (c == '\n') {
				buffer[pos] = 0;
				vga_write_char('\n');
				break;
			} else if (c == '\b') {
				if (pos > 0) {
					pos--;
					vga_write_char('\b');
				}
			} else if (c >= 32 && c < 127) {
				buffer[pos++] = c;
				vga_write_char(c);
			}
		}

		if (pos == 0) continue;

		/* Parse command */
		int argc = parse_command(buffer, argv, MAX_ARGS);
		if (argc == 0) continue;

		/* Execute command */
		int found = 0;
		for (int i = 0; commands[i].name; i++) {
			if (strcmp(argv[0], commands[i].name) == 0) {
				int result = commands[i].handler(argc, argv);
				if (result == -1) return;  /* Exit shell */
				found = 1;
				break;
			}
		}

		if (!found) {
			vga_write_string("Unknown command: ");
			vga_write_string(argv[0]);
			vga_write_string("\n");
		}
	}
}
