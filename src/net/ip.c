#include "net.h"
#include "memory.h"
#include "string.h"

void ip_init(void) {
	/* Initialize IP layer */
}

/* Handle incoming IP packet */
void ip_handle_packet(uint8_t* data, uint16_t len) {
	if (len < sizeof(ipv4_hdr_t)) {
		return;
	}

	ipv4_hdr_t* ip = (ipv4_hdr_t*) data;

	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		return;
	}

	/* Check version */
	if ((ip->version_ihl >> 4) != 4) {
		return;
	}

	/* Check if packet is for us */
	if (memcmp(&ip->dest_addr, &iface->ip_addr, sizeof(ipv4_addr_t)) != 0) {
		/* Check for broadcast? */
		return;
	}

	/* Calculate IHL (Internet Header Length) */
	uint8_t ihl = (ip->version_ihl & 0x0F) * 4;
	if (ihl > len) {
		return;
	}

	uint8_t* payload = data + ihl;
	uint16_t payload_len = net_ntohs(ip->total_length) - ihl;

	/* Route to appropriate protocol handler */
	switch (ip->protocol) {
		case IP_PROTO_ICMP:
			icmp_handle_packet(payload, payload_len);
			break;

		case IP_PROTO_UDP:
			udp_handle_packet(payload, payload_len);
			break;

		case IP_PROTO_TCP:
			tcp_handle_packet(payload, payload_len);
			break;

		default:
			break;
	}
}

/* Send IP packet */
void ip_send_packet(ipv4_addr_t dest, uint8_t proto, uint8_t* data, uint16_t len) {
	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		return;
	}

	/* Allocate buffer for Ethernet + IP + payload */
	uint16_t total_len = sizeof(ethernet_hdr_t) + sizeof(ipv4_hdr_t) + len;
	uint8_t* buffer = malloc(total_len);
	if (!buffer) {
		return;
	}

	/* Build Ethernet header */
	ethernet_hdr_t* eth = (ethernet_hdr_t*) buffer;
	eth->src_mac = iface->mac_addr;
	eth->dest_mac = arp_lookup(dest);
	eth->ethertype = net_htons(ETHERTYPE_IPV4);

	/* Build IP header */
	ipv4_hdr_t* ip = (ipv4_hdr_t*) (buffer + sizeof(ethernet_hdr_t));
	ip->version_ihl = (4 << 4) | 5;  /* Version 4, IHL 5 */
	ip->dscp_ecn = 0;
	ip->total_length = net_htons(sizeof(ipv4_hdr_t) + len);
	ip->identification = 0;
	ip->flags_frag_offset = 0;  /* No fragmentation */
	ip->ttl = 64;
	ip->protocol = proto;
	ip->src_addr = iface->ip_addr;
	ip->dest_addr = dest;

	/* Calculate checksum (set to 0 first) */
	ip->checksum = 0;
	ip->checksum = net_checksum((uint8_t*) ip, sizeof(ipv4_hdr_t));

	/* Copy payload */
	uint8_t* payload = buffer + sizeof(ethernet_hdr_t) + sizeof(ipv4_hdr_t);
	memcpy(payload, data, len);

	/* Send packet */
	net_send_packet(buffer, total_len);
	free(buffer);
}
