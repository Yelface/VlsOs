#include "net.h"
#include "memory.h"

void udp_init(void) {
	/* Initialize UDP layer */
}

void udp_handle_packet(uint8_t* data, uint16_t len) {
	if (len < sizeof(udp_hdr_t)) {
		return;
	}

	udp_hdr_t* udp = (udp_hdr_t*) data;

	/* UDP packet received */
	/* Dispatch to application layer */
	/* For now, just log it */
}

void udp_send_packet(ipv4_addr_t dest, uint16_t src_port, uint16_t dest_port, uint8_t* data, uint16_t len) {
	/* Allocate buffer for UDP + payload */
	uint8_t* buffer = malloc(sizeof(udp_hdr_t) + len);
	if (!buffer) {
		return;
	}

	/* Build UDP header */
	udp_hdr_t* udp = (udp_hdr_t*) buffer;
	udp->src_port = net_htons(src_port);
	udp->dest_port = net_htons(dest_port);
	udp->length = net_htons(sizeof(udp_hdr_t) + len);

	/* Copy payload */
	uint8_t* payload = buffer + sizeof(udp_hdr_t);
	memcpy(payload, data, len);

	/* Calculate checksum (can be omitted for IPv4) */
	udp->checksum = 0;

	/* Send via IP */
	ip_send_packet(dest, IP_PROTO_UDP, buffer, sizeof(udp_hdr_t) + len);
	free(buffer);
}
