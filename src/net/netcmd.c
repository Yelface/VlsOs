#include "drivers.h"
#include "string.h"
#include "types.h"
#include "net.h"
#include "socket.h"

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
