#include "string.h"
#include "types.h"
#include "drivers.h"

/* Simple Search Engine for VlsOs */

/* Search result structure */
struct search_result {
	const char* title;
	const char* content;
	int relevance;
};

/* System data registry */
static const struct search_result system_data[] = {
	{"Command: help", "Display available commands", 1},
	{"Command: echo", "Echo arguments to display text output", 2},
	{"Command: clear", "Clear the screen and reset display", 1},
	{"Command: uptime", "Show system uptime in ticks", 1},
	{"Command: ls", "List files and directories", 1},
	{"Command: exit", "Exit the shell", 1},
	{"Command: ifconfig", "Show network interface configuration and IP", 2},
	{"Command: ping", "Send ICMP echo request ping to host", 2},
	{"Command: netstat", "Show network connections and statistics", 2},
	{"Command: arp", "Show ARP cache and address resolution", 2},
	{"Command: route", "Show routing table", 1},
	{"Command: http", "HTTP server control start stop status", 2},
	{"Command: search", "Search system information and data", 2},
	{"System: Memory", "Virtual memory management and paging", 1},
	{"System: PIT", "Programmable Interval Timer for scheduling", 1},
	{"System: IDT", "Interrupt Descriptor Table for exceptions", 1},
	{"System: Keyboard", "PS/2 keyboard driver and input handling", 1},
	{"System: VGA", "Video Graphics Array text mode display", 1},
	{"System: Network", "Network stack with TCP IP UDP support", 2},
	{"System: ARP", "Address Resolution Protocol for MAC lookup", 1},
	{"System: ICMP", "Internet Control Message Protocol ping", 1},
	{"System: TCP", "Transmission Control Protocol reliable stream", 2},
	{"System: UDP", "User Datagram Protocol unreliable datagram", 1},
	{"System: Socket", "Socket abstraction for network communication", 2},
	{"System: HTTP", "HyperText Transfer Protocol web server", 2},
	{"Architecture: x86", "32-bit x86 processor instruction set", 1},
	{"Architecture: Multiboot", "Multiboot bootloader specification", 1},
	{"Architecture: Interrupts", "Interrupt handling and exception dispatch", 1},
	{NULL, NULL, 0}
};

/* Simple substring search with case-insensitive matching */
static int contains_pattern(const char* text, const char* pattern) {
	if (!text || !pattern) return 0;
	
	int tlen = strlen(text);
	int plen = strlen(pattern);
	
	if (plen > tlen) return 0;
	
	/* Case-insensitive search */
	for (int i = 0; i <= tlen - plen; i++) {
		int match = 1;
		for (int j = 0; j < plen; j++) {
			char t = text[i + j];
			char p = pattern[j];
			
			/* Convert to lowercase for comparison */
			if (t >= 'A' && t <= 'Z') t += 32;
			if (p >= 'A' && p <= 'Z') p += 32;
			
			if (t != p) {
				match = 0;
				break;
			}
		}
		if (match) return 1;
	}
	
	return 0;
}

/* Calculate relevance score based on match position and type */
static int calculate_relevance(const char* title, const char* content, const char* pattern) {
	int score = 0;
	
	/* Title match is more relevant */
	if (contains_pattern(title, pattern)) {
		score += 10;
		/* Exact word match at start of title */
		if (strncmp(title, pattern, strlen(pattern)) == 0 || 
		    strncmp(title + 2, pattern, strlen(pattern)) == 0) {
			score += 20;
		}
	}
	
	/* Content match */
	if (contains_pattern(content, pattern)) {
		score += 5;
	}
	
	return score;
}

/* Search system data */
int search_system(const char* query) {
	if (!query || *query == 0) {
		vga_write_string("Usage: search <query>\n");
		vga_write_string("Example: search network\n");
		return 1;
	}
	
	vga_write_string("=== Search Results for: ");
	vga_write_string(query);
	vga_write_string(" ===\n\n");
	
	int results_found = 0;
	
	/* First pass: find all matches and calculate relevance */
	struct search_result matches[32];
	int match_count = 0;
	
	for (int i = 0; system_data[i].title != NULL; i++) {
		int relevance = calculate_relevance(
			system_data[i].title,
			system_data[i].content,
			query
		);
		
		if (relevance > 0 && match_count < 32) {
			matches[match_count].title = system_data[i].title;
			matches[match_count].content = system_data[i].content;
			matches[match_count].relevance = relevance;
			match_count++;
		}
	}
	
	/* Sort by relevance (simple bubble sort) */
	for (int i = 0; i < match_count - 1; i++) {
		for (int j = 0; j < match_count - i - 1; j++) {
			if (matches[j].relevance < matches[j + 1].relevance) {
				struct search_result temp = matches[j];
				matches[j] = matches[j + 1];
				matches[j + 1] = temp;
			}
		}
	}
	
	/* Display results */
	if (match_count == 0) {
		vga_write_string("No results found.\n");
		return 1;
	}
	
	for (int i = 0; i < match_count; i++) {
		vga_write_string("[");
		char num_buf[4];
		itoa(i + 1, num_buf, 10);
		vga_write_string(num_buf);
		vga_write_string("] ");
		vga_write_string(matches[i].title);
		vga_write_string("\n    ");
		vga_write_string(matches[i].content);
		vga_write_string("\n\n");
		results_found++;
	}
	
	vga_write_string("Found ");
	char count_buf[4];
	itoa(results_found, count_buf, 10);
	vga_write_string(count_buf);
	vga_write_string(" result(s).\n");
	
	return 0;
}
