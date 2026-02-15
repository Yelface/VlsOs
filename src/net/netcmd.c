#include "drivers.h"
#include "string.h"
#include "types.h"
#include "disk.h"
#include "process.h"
#include "filesystem.h"
#include "ipc.h"
#include "net.h"
#include "socket.h"
#include "http.h"
#include "dns.h"
#include "dhcp.h"
#include "http_client.h"
#include "ui.h"

/* Network commands for shell */

int cmd_ifconfig(int argc, char** argv) {
	(void) argc;
	(void) argv;

	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		vga_write_string("No network interface found\n");
		return 0;
	}

	vga_write_string("eth0: ");

	/* Print MAC address */
	vga_write_string("MAC=");
	char buf[3];
	for (int i = 0; i < 6; i++) {
		itoa(iface->mac_addr.octets[i], buf, 16);
		if (buf[1] == 0) vga_write_char('0');
		vga_write_string(buf);
		if (i < 5) vga_write_char(':');
	}
	vga_write_string("\n");

	/* Print IP address */
	vga_write_string("      inet ");
	for (int i = 0; i < 4; i++) {
		itoa(iface->ip_addr.octets[i], buf, 10);
		vga_write_string(buf);
		if (i < 3) vga_write_char('.');
	}
	vga_write_string(" ");

	/* Print netmask */
	vga_write_string("netmask ");
	for (int i = 0; i < 4; i++) {
		itoa(iface->netmask.octets[i], buf, 10);
		vga_write_string(buf);
		if (i < 3) vga_write_char('.');
	}
	vga_write_string("\n");

	/* Print gateway */
	vga_write_string("      gateway ");
	for (int i = 0; i < 4; i++) {
		itoa(iface->gateway.octets[i], buf, 10);
		vga_write_string(buf);
		if (i < 3) vga_write_char('.');
	}
	vga_write_string("\n");

	/* Print DNS servers */
	vga_write_string("      dns1 ");
	for (int i = 0; i < 4; i++) {
		itoa(iface->dns1.octets[i], buf, 10);
		vga_write_string(buf);
		if (i < 3) vga_write_char('.');
	}
	vga_write_string(" dns2 ");
	for (int i = 0; i < 4; i++) {
		itoa(iface->dns2.octets[i], buf, 10);
		vga_write_string(buf);
		if (i < 3) vga_write_char('.');
	}
	vga_write_string("\n");

	return 0;
}

int cmd_ping(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: ping <ip_address>\n");
		return 0;
	}

	/* Parse IP address */
	ipv4_addr_t addr;
	int octets[4];
	char* arg = argv[1];
	int octet_idx = 0;

	/* Simple IP parser (e.g., "192.168.1.1") */
	char num_buf[4] = {0};
	int num_idx = 0;

	for (int i = 0; arg[i] && octet_idx < 4; i++) {
		if (arg[i] == '.' || arg[i] == 0) {
			if (num_idx > 0) {
				octets[octet_idx++] = atoi(num_buf);
				num_idx = 0;
				num_buf[0] = 0;
			}
		} else if (isdigit(arg[i]) && num_idx < 3) {
			num_buf[num_idx++] = arg[i];
			num_buf[num_idx] = 0;
		}
	}

	if (octet_idx < 4) {
		octets[octet_idx++] = atoi(num_buf);
	}

	if (octet_idx != 4) {
		vga_write_string("Invalid IP address\n");
		return 0;
	}

	addr.octets[0] = octets[0];
	addr.octets[1] = octets[1];
	addr.octets[2] = octets[2];
	addr.octets[3] = octets[3];

	/* Send ICMP echo request */
	vga_write_string("PING ");
	vga_write_string(argv[1]);
	vga_write_string("\n");

	icmp_send_echo_request(addr);

	return 0;
}

int cmd_netstat(int argc, char** argv) {
	(void) argc;
	(void) argv;

	vga_write_string("TCP/UDP connections (not yet implemented)\n");
	return 0;
}

int cmd_arp(int argc, char** argv) {
	(void) argc;
	(void) argv;

	vga_write_string("ARP cache (not yet implemented)\n");
	return 0;
}

int cmd_route(int argc, char** argv) {
	(void) argc;
	(void) argv;

	vga_write_string("Routing table (not yet implemented)\n");
	return 0;
}

/* HTTP Server command */
int cmd_http(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: http start|stop|status\n");
		return 1;
	}

	if (strcmp(argv[1], "start") == 0) {
		http_server_start();
	} else if (strcmp(argv[1], "stop") == 0) {
		http_server_stop();
	} else if (strcmp(argv[1], "status") == 0) {
		vga_write_string("HTTP server status (not yet implemented)\n");
	} else {
		vga_write_string("Unknown http command\n");
		return 1;
	}

	return 0;
}

/* DNS Server command */
int cmd_dns(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: dns start|stop|status\n");
		return 1;
	}

	if (strcmp(argv[1], "start") == 0) {
		dns_start();
	} else if (strcmp(argv[1], "stop") == 0) {
		dns_stop();
	} else if (strcmp(argv[1], "status") == 0) {
		vga_write_string("DNS server status (not yet implemented)\n");
	} else {
		vga_write_string("Unknown dns command\n");
		return 1;
	}

	return 0;
}

/* DHCP Server command */
int cmd_dhcp(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: dhcp start|stop|status\n");
		return 1;
	}

	if (strcmp(argv[1], "start") == 0) {
		dhcp_start();
	} else if (strcmp(argv[1], "stop") == 0) {
		dhcp_stop();
	} else if (strcmp(argv[1], "status") == 0) {
		vga_write_string("DHCP server status (not yet implemented)\n");
	} else {
		vga_write_string("Unknown dhcp command\n");
		return 1;
	}

	return 0;
}
/* HTTP Client - wget command */
int cmd_wget(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: wget <url>\n");
		vga_write_string("Example: wget http://192.168.1.100/\n");
		return 1;
	}

	const char* url = argv[1];
	
	/* Simple URL parser: http://host:port/path */
	const char* host_start = url;
	if (strncmp(url, "http://", 7) == 0) {
		host_start = url + 7;
	} else {
		vga_write_string("Only http:// URLs supported\n");
		return 1;
	}

	/* Extract host and path */
	char host[64] = {0};
	uint16_t port = 80;
	const char* path = "/";
	int host_len = 0;

	/* Copy host until : or / */
	int i = 0;
	while (host_start[i] && host_start[i] != ':' && host_start[i] != '/' && i < 63) {
		host[i] = host_start[i];
		i++;
		host_len++;
	}
	host[host_len] = 0;

	/* Check for port */
	if (host_start[i] == ':') {
		i++;
		char port_str[6] = {0};
		int p_idx = 0;
		while (host_start[i] && host_start[i] >= '0' && host_start[i] <= '9' && p_idx < 5) {
			port_str[p_idx++] = host_start[i++];
		}
		port_str[p_idx] = 0;
		if (p_idx > 0) port = atoi(port_str);
	}

	/* Rest is path */
	if (host_start[i] == '/') {
		path = &host_start[i];
	}

	vga_write_string("Fetching ");
	vga_write_string(host);
	vga_write_string(":");
	char portstr[6]; itoa(port, portstr, 10);
	vga_write_string(portstr);
	vga_write_string(path);
	vga_write_string(" ...\n");

	/* Make HTTP request */
	http_response_t resp;
	if (http_client_get(host, port, path, &resp) < 0) {
		return 1;
	}

	vga_write_string("Status: ");
	char status_str[4]; itoa(resp.status_code, status_str, 10);
	vga_write_string(status_str);
	vga_write_string("\n");

	if (resp.body) {
		vga_write_string("Content:\n");
		vga_write_string("---\n");
		for (int j = 0; j < resp.body_len; j++) {
			vga_write_char(resp.body[j]);
		}
		vga_write_string("\n---\n");
		http_client_free(&resp);
	}

	return 0;
}

/* Disk command */
int cmd_disk(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: disk info|read|write\n");
		vga_write_string("  disk info              - Show disk information\n");
		vga_write_string("  disk read <drive> <lba> - Read sector from disk\n");
		return 1;
	}

	if (strcmp(argv[1], "info") == 0) {
		uint8_t drive_count = disk_get_drive_count();
		vga_write_string("Disk subsystem: ");
		char buf[32];
		itoa(drive_count, buf, 10);
		vga_write_string(buf);
		vga_write_string(" drive(s) detected\n\n");

		for (uint8_t i = 0; i < 4; i++) {
			if (disk_drive_exists(i)) {
				ata_disk_t* disk = disk_get_info(i);
				if (disk) {
					vga_write_string("Drive ");
					itoa(i, buf, 10);
					vga_write_string(buf);
					vga_write_string(":\n");
					vga_write_string("  Model: ");
					vga_write_string(disk->model);
					vga_write_string("\n");
					vga_write_string("  Cylinders: ");
					itoa(disk->cylinders, buf, 10);
					vga_write_string(buf);
					vga_write_string("  Heads: ");
					itoa(disk->heads, buf, 10);
					vga_write_string(buf);
					vga_write_string("  Sectors/track: ");
					itoa(disk->sectors, buf, 10);
					vga_write_string(buf);
					vga_write_string("\n");
					vga_write_string("  Total sectors: ");
					itoa(disk->total_sectors, buf, 10);
					vga_write_string(buf);
					vga_write_string("  (");
					uint32_t mb = (disk->total_sectors * 512) / (1024 * 1024);
					itoa(mb, buf, 10);
					vga_write_string(buf);
					vga_write_string(" MB)\n\n");
				}
			}
		}
		return 0;
	} else if (strcmp(argv[1], "read") == 0) {
		if (argc < 4) {
			vga_write_string("Usage: disk read <drive> <lba>\n");
			return 1;
		}
		
		uint8_t drive = (uint8_t)atoi(argv[2]);
		uint32_t lba = (uint32_t)atoi(argv[3]);
		
		if (!disk_drive_exists(drive)) {
			vga_write_string("Drive not found\n");
			return 1;
		}

		uint8_t buffer[512];
		int result = disk_read_sector(drive, lba, buffer);
		if (result < 0) {
			vga_write_string("Read failed\n");
			return 1;
		}

		vga_write_string("Sector 0x");
		char buf[32];
		itoa(lba, buf, 16);
		vga_write_string(buf);
		vga_write_string(" (");
		itoa(lba, buf, 10);
		vga_write_string(buf);
		vga_write_string("):\n");

		/* Display first 256 bytes */
		for (int i = 0; i < 256; i++) {
			if (i % 16 == 0) {
				vga_write_string("\n0x");
				itoa(i, buf, 16);
				vga_write_string(buf);
				vga_write_string(": ");
			}
			uint8_t byte = buffer[i];
			if (byte < 16) vga_write_char('0');
			itoa(byte, buf, 16);
			vga_write_string(buf);
			vga_write_char(' ');
		}
		vga_write_string("\n");
		return 0;
	} else if (strcmp(argv[1], "write") == 0) {
		vga_write_string("Write command not implemented\n");
		return 1;
	} else {
		vga_write_string("Unknown disk command\n");
		return 1;
	}
}

/* List directory command */
int cmd_ls(int argc, char** argv) {
	(void)argc;
	(void)argv;

	fs_dir_info_t entries[FS_ROOT_ENTRIES];
	int count = fs_list_dir("", entries, FS_ROOT_ENTRIES);

	if (count < 0) {
		vga_write_string("Failed to list directory\n");
		return 1;
	}

	char buf[32];
	vga_write_string("NAME             SIZE      ATTR\n");
	vga_write_string("================ ========= ======\n");

	for (int i = 0; i < count; i++) {
		/* Name */
		vga_write_string(entries[i].filename);
		for (int j = strlen(entries[i].filename); j < 16; j++) {
			vga_write_char(' ');
		}

		/* Size */
		itoa(entries[i].file_size, buf, 10);
		vga_write_string(buf);
		for (int j = strlen(buf); j < 9; j++) {
			vga_write_char(' ');
		}

		/* Attributes */
		if (entries[i].attributes & FS_ATTR_DIRECTORY) {
			vga_write_string("<DIR>");
		} else {
			vga_write_string("-");
		}
		vga_write_char('\n');
	}

	vga_write_string("\nTotal: ");
	itoa(count, buf, 10);
	vga_write_string(buf);
	vga_write_string(" entries\n");

	return 0;
}

/* Display file command */
int cmd_cat(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: cat <filename>\n");
		return 1;
	}

	if (!fs_file_exists(argv[1])) {
		vga_write_string("File not found: ");
		vga_write_string(argv[1]);
		vga_write_char('\n');
		return 1;
	}

	int fd = fs_open(argv[1], 0);
	if (fd < 0) {
		vga_write_string("Failed to open file\n");
		return 1;
	}

	uint8_t buffer[256];
	int bytes_read;

	while ((bytes_read = fs_read(fd, buffer, sizeof(buffer))) > 0) {
		for (int i = 0; i < bytes_read; i++) {
			vga_write_char((char)buffer[i]);
		}
	}

	fs_close(fd);
	vga_write_char('\n');
	return 0;
}

/* File information command */
int cmd_file(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: file <filename>\n");
		return 1;
	}

	fs_dir_info_t info;
	if (fs_get_file_info(argv[1], &info) < 0) {
		vga_write_string("File not found\n");
		return 1;
	}

	char buf[32];
	vga_write_string("File: ");
	vga_write_string(info.filename);
	vga_write_char('\n');
	
	vga_write_string("Size: ");
	itoa(info.file_size, buf, 10);
	vga_write_string(buf);
	vga_write_string(" bytes\n");
	
	vga_write_string("Start cluster: ");
	itoa(info.start_cluster, buf, 10);
	vga_write_string(buf);
	vga_write_char('\n');
	
	vga_write_string("Attributes: ");
	itoa(info.attributes, buf, 16);
	vga_write_string(buf);
	vga_write_char('\n');

	return 0;
}

/* Pipe test command */
int cmd_pipe(int argc, char** argv) {
	(void)argc;
	(void)argv;

	vga_write_string("Creating pipe...\n");
	int pipe_id = ipc_pipe_create();

	if (pipe_id < 0) {
		vga_write_string("Failed to create pipe\n");
		return 1;
	}

	char buf[32];
	vga_write_string("Pipe created: ID=");
	itoa(pipe_id, buf, 10);
	vga_write_string(buf);
	vga_write_char('\n');

	/* Test write */
	vga_write_string("Writing test message...\n");
	ipc_pipe_write(pipe_id, 0x12345678);
	ipc_pipe_write(pipe_id, 0xDEADBEEF);

	/* Test read */
	uint32_t msg1, msg2;
	ipc_pipe_read(pipe_id, &msg1);
	ipc_pipe_read(pipe_id, &msg2);

	vga_write_string("Read from pipe: 0x");
	itoa(msg1, buf, 16);
	vga_write_string(buf);
	vga_write_string(" and 0x");
	itoa(msg2, buf, 16);
	vga_write_string(buf);
	vga_write_char('\n');

	ipc_pipe_close(pipe_id);
	vga_write_string("Pipe closed\n");

	return 0;
}

/* UI command */
int cmd_ui(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: ui on|off|status\n");
		return 1;
	}

	if (strcmp(argv[1], "on") == 0) {
		ui_enable();
		ui_update_header();
		ui_print_content("Enhanced UI enabled");
		ui_draw_footer("vlsos> ");
	} else if (strcmp(argv[1], "off") == 0) {
		ui_disable();
		vga_write_string("UI disabled\n");
	} else if (strcmp(argv[1], "status") == 0) {
		vga_write_string("UI status: ");
		vga_write_string(g_ui_state.enabled ? "ON\n" : "OFF\n");
	} else {
		vga_write_string("Unknown ui command\n");
		return 1;
	}

	return 0;
}

/* Process list command */
int cmd_ps(int argc, char** argv) {
	(void)argc;
	(void)argv;
	
	process_display_info();
	return 0;
}

/* Kill process command */
int cmd_kill(int argc, char** argv) {
	if (argc < 2) {
		vga_write_string("Usage: kill <pid>\n");
		return 1;
	}

	uint32_t pid = (uint32_t)atoi(argv[1]);
	
	if (pid == 0) {
		vga_write_string("Cannot kill kernel process\n");
		return 1;
	}

	process_t* proc = process_get(pid);
	if (!proc) {
		vga_write_string("Process not found\n");
		return 1;
	}

	char buf[32];
	int result = process_kill(pid);
	if (result == 0) {
		vga_write_string("Killed process ");
		itoa(pid, buf, 10);
		vga_write_string(buf);
		vga_write_string("\n");
	} else {
		vga_write_string("Failed to kill process\n");
		return 1;
	}

	return 0;
}