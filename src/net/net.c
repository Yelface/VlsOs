#include "net.h"
#include "memory.h"
#include "string.h"
#include "drivers.h"
#include "types.h"

/* Network interfaces (support single interface for now) */
static net_interface_t interfaces[1];
static int num_interfaces = 0;

/* ARP cache */
struct arp_entry {
	ipv4_addr_t ip;
	mac_addr_t mac;
	uint32_t timestamp;
	int valid;
};

static struct arp_entry arp_cache[32];
static int arp_cache_size = 0;

/* Initialize networking subsystem */
void net_init(void) {
	vga_write_string("Initializing network stack...\n");

	/* Initialize protocols */
	arp_init();
	ip_init();
	icmp_init();
	udp_init();
	tcp_init();

	vga_write_string("Network stack initialized\n");
}

/* Network polling (called periodically) */
void net_poll(void) {
	/* Poll each interface for incoming packets */
	for (int i = 0; i < num_interfaces; i++) {
		if (interfaces[i].receive && (interfaces[i].flags & NET_FLAG_RUNNING)) {
			interfaces[i].receive();
		}
	}
}

/* Get network interface by name */
net_interface_t* net_get_interface(const char* name) {
	for (int i = 0; i < num_interfaces; i++) {
		if (strcmp(interfaces[i].name, name) == 0) {
			return &interfaces[i];
		}
	}
	return NULL;
}

/* Register network interface */
net_interface_t* net_register_interface(const char* name) {
	if (num_interfaces >= 1) {
		return NULL;  /* Only support 1 interface for now */
	}

	net_interface_t* iface = &interfaces[num_interfaces++];
	iface->name = name;
	iface->mtu = NET_MTU;
	iface->flags = NET_FLAG_UP;

	return iface;
}

/* Set IP address */
void net_set_ipaddr(net_interface_t* iface, ipv4_addr_t addr) {
	iface->ip_addr = addr;
}

/* Set netmask */
void net_set_netmask(net_interface_t* iface, ipv4_addr_t mask) {
	iface->netmask = mask;
}

/* Receive frame from driver */
void net_receive_frame(uint8_t* data, uint16_t len) {
	if (len < sizeof(ethernet_hdr_t)) {
		return;
	}

	ethernet_hdr_t* eth = (ethernet_hdr_t*) data;

	switch (net_ntohs(eth->ethertype)) {
		case ETHERTYPE_ARP:
			arp_handle_packet(data + sizeof(ethernet_hdr_t), len - sizeof(ethernet_hdr_t));
			break;

		case ETHERTYPE_IPV4:
			ip_handle_packet(data + sizeof(ethernet_hdr_t), len - sizeof(ethernet_hdr_t));
			break;

		default:
			break;
	}
}

/* Send packet with Ethernet frame */
void net_send_packet(uint8_t* data, uint16_t len) {
	if (num_interfaces == 0) {
		return;
	}

	net_interface_t* iface = &interfaces[0];

	if (iface->send && (iface->flags & NET_FLAG_UP)) {
		iface->send(data, len);
	}
}

/* Calculate network checksum */
uint16_t net_checksum(uint8_t* data, uint32_t len) {
	uint32_t sum = 0;
	uint16_t* words = (uint16_t*) data;

	for (uint32_t i = 0; i < len / 2; i++) {
		sum += net_ntohs(words[i]);
	}

	/* Handle odd byte */
	if (len % 2) {
		sum += data[len - 1];
	}

	/* Add carry bits */
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ~sum;
}

/* byte order conversion functions */
uint16_t net_htons(uint16_t value) {
	return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
}

uint16_t net_ntohs(uint16_t value) {
	return net_htons(value);
}

uint32_t net_htonl(uint32_t value) {
	return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
	       ((value >> 8) & 0xFF00) | ((value >> 24) & 0xFF);
}

uint32_t net_ntohl(uint32_t value) {
	return net_htonl(value);
}

/* Set IPv4 address from octets */
void net_set_ipaddr_bytes(ipv4_addr_t* addr, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	addr->octets[0] = a;
	addr->octets[1] = b;
	addr->octets[2] = c;
	addr->octets[3] = d;
}

/* Set MAC address from octets */
void net_set_macaddr_bytes(mac_addr_t* addr, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
	addr->octets[0] = a;
	addr->octets[1] = b;
	addr->octets[2] = c;
	addr->octets[3] = d;
	addr->octets[4] = e;
	addr->octets[5] = f;
}
