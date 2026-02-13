#include "net.h"
#include "memory.h"
#include "string.h"
#include "drivers.h"

/* ARP cache */
typedef struct {
	ipv4_addr_t ip;
	mac_addr_t mac;
	uint32_t age;
	int valid;
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[32];
static int arp_cache_entries = 0;

/* Forward declarations */
static void arp_send_reply(ipv4_addr_t dest_ip, mac_addr_t dest_mac);
void arp_cache_learn(ipv4_addr_t ip, mac_addr_t mac);

void arp_init(void) {
	memset(arp_cache, 0, sizeof(arp_cache));
	arp_cache_entries = 0;
}

/* Handle incoming ARP packet */
void arp_handle_packet(uint8_t* data, uint16_t len) {
	if (len < sizeof(arp_pkt_t)) {
		return;
	}

	arp_pkt_t* arp = (arp_pkt_t*) data;

	/* Only handle Ethernet + IPv4 ARP */
	if (net_ntohs(arp->hw_type) != 1 || net_ntohs(arp->proto_type) != ETHERTYPE_IPV4) {
		return;
	}

	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		return;
	}

	/* Add sender to ARP cache */
	arp_cache_learn(arp->sender_ip, arp->sender_mac);

	/* Handle ARP request */
	if (net_ntohs(arp->opcode) == ARP_OP_REQUEST) {
		/* Check if request is for our IP */
		if (memcmp(&arp->target_ip, &iface->ip_addr, sizeof(ipv4_addr_t)) == 0) {
			arp_send_reply(arp->sender_ip, arp->sender_mac);
		}
	}
}

/* Send ARP request */
void arp_request(ipv4_addr_t target_ip) {
	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		return;
	}

	/* Allocate buffer for Ethernet + ARP */
	uint8_t* buffer = malloc(sizeof(ethernet_hdr_t) + sizeof(arp_pkt_t));
	if (!buffer) {
		return;
	}

	/* Build Ethernet header */
	ethernet_hdr_t* eth = (ethernet_hdr_t*) buffer;
	memset(&eth->dest_mac, 0xFF, sizeof(mac_addr_t));  /* Broadcast */
	eth->src_mac = iface->mac_addr;
	eth->ethertype = net_htons(ETHERTYPE_ARP);

	/* Build ARP request */
	arp_pkt_t* arp = (arp_pkt_t*) (buffer + sizeof(ethernet_hdr_t));
	arp->hw_type = net_htons(1);  /* Ethernet */
	arp->proto_type = net_htons(ETHERTYPE_IPV4);
	arp->hw_addr_len = 6;
	arp->proto_addr_len = 4;
	arp->opcode = net_htons(ARP_OP_REQUEST);

	arp->sender_mac = iface->mac_addr;
	arp->sender_ip = iface->ip_addr;
	memset(&arp->target_mac, 0, sizeof(mac_addr_t));
	arp->target_ip = target_ip;

	/* Send packet */
	net_send_packet(buffer, sizeof(ethernet_hdr_t) + sizeof(arp_pkt_t));
	free(buffer);
}

/* Send ARP reply */
static void arp_send_reply(ipv4_addr_t dest_ip, mac_addr_t dest_mac) {
	net_interface_t* iface = net_get_interface("eth0");
	if (!iface) {
		return;
	}

	/* Allocate buffer */
	uint8_t* buffer = malloc(sizeof(ethernet_hdr_t) + sizeof(arp_pkt_t));
	if (!buffer) {
		return;
	}

	/* Build Ethernet header */
	ethernet_hdr_t* eth = (ethernet_hdr_t*) buffer;
	eth->dest_mac = dest_mac;
	eth->src_mac = iface->mac_addr;
	eth->ethertype = net_htons(ETHERTYPE_ARP);

	/* Build ARP reply */
	arp_pkt_t* arp = (arp_pkt_t*) (buffer + sizeof(ethernet_hdr_t));
	arp->hw_type = net_htons(1);
	arp->proto_type = net_htons(ETHERTYPE_IPV4);
	arp->hw_addr_len = 6;
	arp->proto_addr_len = 4;
	arp->opcode = net_htons(ARP_OP_REPLY);

	arp->sender_mac = iface->mac_addr;
	arp->sender_ip = iface->ip_addr;
	arp->target_mac = dest_mac;
	arp->target_ip = dest_ip;

	/* Send packet */
	net_send_packet(buffer, sizeof(ethernet_hdr_t) + sizeof(arp_pkt_t));
	free(buffer);
}

/* Learn MAC address from IP */
void arp_cache_learn(ipv4_addr_t ip, mac_addr_t mac) {
	/* Check if already in cache */
	for (int i = 0; i < arp_cache_entries; i++) {
		if (memcmp(&arp_cache[i].ip, &ip, sizeof(ipv4_addr_t)) == 0) {
			arp_cache[i].mac = mac;
			arp_cache[i].valid = 1;
			return;
		}
	}

	/* Add new entry */
	if (arp_cache_entries < 32) {
		arp_cache[arp_cache_entries].ip = ip;
		arp_cache[arp_cache_entries].mac = mac;
		arp_cache[arp_cache_entries].valid = 1;
		arp_cache_entries++;
	}
}

/* Look up MAC address from IP */
mac_addr_t arp_lookup(ipv4_addr_t ip) {
	for (int i = 0; i < arp_cache_entries; i++) {
		if (arp_cache[i].valid && memcmp(&arp_cache[i].ip, &ip, sizeof(ipv4_addr_t)) == 0) {
			return arp_cache[i].mac;
		}
	}

	/* Not found - send ARP request and wait */
	arp_request(ip);

	/* Return broadcast for now */
	mac_addr_t broadcast;
	memset(&broadcast, 0xFF, sizeof(mac_addr_t));
	return broadcast;
}
