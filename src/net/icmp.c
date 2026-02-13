#include "net.h"
#include "memory.h"
#include "string.h"
#include "drivers.h"

/* Forward declarations */
static void icmp_send_echo_reply(uint8_t* data, uint16_t len);

void icmp_init(void) {
	/* Initialize ICMP layer */
}

/* Handle incoming ICMP packet */
void icmp_handle_packet(uint8_t* data, uint16_t len) {
	if (len < sizeof(icmp_hdr_t)) {
		return;
	}

	icmp_hdr_t* icmp = (icmp_hdr_t*) data;

	/* Handle echo request (ping) */
	if (icmp->type == ICMP_ECHO_REQUEST) {
		icmp_send_echo_reply(data, len);
	}
}

/* Send ICMP echo request (ping) */
void icmp_send_echo_request(ipv4_addr_t dest) {
	icmp_hdr_t* icmp = malloc(sizeof(icmp_hdr_t) + 32);
	if (!icmp) {
		return;
	}

	icmp->type = ICMP_ECHO_REQUEST;
	icmp->code = 0;
	icmp->id = net_htons(0x1234);
	icmp->seq = net_htons(1);

	/* Add some data */
	uint8_t* data = (uint8_t*) icmp + sizeof(icmp_hdr_t);
	for (int i = 0; i < 32; i++) {
		data[i] = i;
	}

	/* Calculate checksum */
	icmp->checksum = 0;
	icmp->checksum = net_checksum((uint8_t*) icmp, sizeof(icmp_hdr_t) + 32);

	/* Send via IP */
	ip_send_packet(dest, IP_PROTO_ICMP, (uint8_t*) icmp, sizeof(icmp_hdr_t) + 32);
	free(icmp);
}

/* Send ICMP echo reply */
static void icmp_send_echo_reply(uint8_t* data, uint16_t len) {
	icmp_hdr_t* icmp_req = (icmp_hdr_t*) data;

	/* Allocate buffer for reply */
	uint8_t* buffer = malloc(len);
	if (!buffer) {
		return;
	}

	memcpy(buffer, data, len);

	/* Modify header */
	icmp_hdr_t* icmp = (icmp_hdr_t*) buffer;
	icmp->type = ICMP_ECHO_REPLY;
	icmp->checksum = 0;
	icmp->checksum = net_checksum(buffer, len);

	/* Get source IP from IP header (passed in context) */
	/* For now, we need to extract it from the packet context */
	/* This is a simplification - in real implementation, pass source IP */

	vga_write_string("ICMP Echo Reply\n");

	free(buffer);
}
