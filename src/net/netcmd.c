#include "drivers.h"
#include "string.h"
#include "types.h"
#include "net.h"
#include "socket.h"
#include "http.h"
#include "dns.h"
#include "dhcp.h"

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
