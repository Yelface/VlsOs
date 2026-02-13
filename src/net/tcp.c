#include "net.h"
#include "memory.h"

void tcp_init(void) {
	/* Initialize TCP layer */
}

void tcp_handle_packet(uint8_t* data, uint16_t len) {
	if (len < sizeof(tcp_hdr_t)) {
		return;
	}

	tcp_hdr_t* tcp = (tcp_hdr_t*) data;

	/* TCP packet received */
	/* Would implement TCP state machine here */
	/* For now, just acknowledge */
}

void tcp_send_packet(ipv4_addr_t dest, uint16_t src_port, uint16_t dest_port,
                     uint32_t seq_num, uint32_t ack_num, uint8_t flags,
                     uint8_t* data, uint16_t len) {
	/* Allocate buffer for TCP + payload */
	uint8_t* buffer = malloc(sizeof(tcp_hdr_t) + len);
	if (!buffer) {
		return;
	}

	/* Build TCP header */
	tcp_hdr_t* tcp = (tcp_hdr_t*) buffer;
	tcp->src_port = net_htons(src_port);
	tcp->dest_port = net_htons(dest_port);
	tcp->seq_num = net_htonl(seq_num);
	tcp->ack_num = net_htonl(ack_num);
	tcp->data_offset = (5 << 4);  /* 5 * 32-bit words, no options */
	tcp->flags = flags;
	tcp->window_size = net_htons(65535);  /* Max window size */
	tcp->urgent_pointer = 0;

	/* Copy payload */
	uint8_t* payload = buffer + sizeof(tcp_hdr_t);
	memcpy(payload, data, len);

	/* Calculate checksum (pseudo-header required) */
	tcp->checksum = 0;
	tcp->checksum = net_checksum(buffer, sizeof(tcp_hdr_t) + len);

	/* Send via IP */
	ip_send_packet(dest, IP_PROTO_TCP, buffer, sizeof(tcp_hdr_t) + len);
	free(buffer);
}
