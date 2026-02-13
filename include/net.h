#ifndef NET_H
#define NET_H

#include "types.h"

/* Network configuration */
#define NET_RX_BUFFERS      32          /* RX ring buffer size */
#define NET_TX_BUFFERS      32          /* TX ring buffer size */
#define NET_MTU             1500        /* Maximum Transmission Unit */
#define NET_BUFFER_SIZE     2048        /* Network buffer size */

/* Network data types */
typedef struct {
	uint8_t octets[6];
} mac_addr_t;

typedef struct {
	uint8_t octets[4];
} ipv4_addr_t;

/* Ethernet frame structure */
typedef struct {
	mac_addr_t dest_mac;
	mac_addr_t src_mac;
	uint16_t ethertype;
} __attribute__((packed)) ethernet_hdr_t;

#define ETHERTYPE_IPV4  0x0800
#define ETHERTYPE_ARP   0x0806

/* ARP packet structure */
typedef struct {
	uint16_t hw_type;       /* Hardware type (1 = Ethernet) */
	uint16_t proto_type;    /* Protocol type (0x0800 = IPv4) */
	uint8_t hw_addr_len;    /* Hardware address length (6) */
	uint8_t proto_addr_len; /* Protocol address length (4) */
	uint16_t opcode;        /* Operation (1=request, 2=reply) */
	mac_addr_t sender_mac;
	ipv4_addr_t sender_ip;
	mac_addr_t target_mac;
	ipv4_addr_t target_ip;
} __attribute__((packed)) arp_pkt_t;

#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

/* IPv4 header structure */
typedef struct {
	uint8_t version_ihl;        /* Version (4 bits) + IHL (4 bits) */
	uint8_t dscp_ecn;           /* DSCP (6 bits) + ECN (2 bits) */
	uint16_t total_length;
	uint16_t identification;
	uint16_t flags_frag_offset; /* Flags (3 bits) + Fragment offset (13 bits) */
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	ipv4_addr_t src_addr;
	ipv4_addr_t dest_addr;
} __attribute__((packed)) ipv4_hdr_t;

#define IP_PROTO_ICMP   1
#define IP_PROTO_TCP    6
#define IP_PROTO_UDP    17

/* ICMP header structure */
typedef struct {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq;
} __attribute__((packed)) icmp_hdr_t;

#define ICMP_ECHO_REQUEST  8
#define ICMP_ECHO_REPLY    0

/* UDP header structure */
typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__((packed)) udp_hdr_t;

/* TCP header structure */
typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq_num;
	uint32_t ack_num;
	uint8_t data_offset;        /* Data offset + reserved (4 bits each) */
	uint8_t flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent_pointer;
} __attribute__((packed)) tcp_hdr_t;

#define TCP_FLAG_FIN    0x01
#define TCP_FLAG_SYN    0x02
#define TCP_FLAG_RST    0x04
#define TCP_FLAG_PSH    0x08
#define TCP_FLAG_ACK    0x10
#define TCP_FLAG_URG    0x20

/* Network packet buffer */
typedef struct {
	uint8_t data[NET_BUFFER_SIZE];
	uint16_t length;
	uint16_t offset;
} net_buffer_t;

/* Network interface structure */
typedef struct {
	const char* name;
	mac_addr_t mac_addr;
	ipv4_addr_t ip_addr;
	ipv4_addr_t netmask;
	ipv4_addr_t gateway;
	uint32_t mtu;
	uint32_t flags;
	void (*send)(uint8_t* data, uint16_t len);
	void (*receive)(void);
} net_interface_t;

#define NET_FLAG_UP     1
#define NET_FLAG_RUNNING 2

/* Network functions */
void net_init(void);
void net_poll(void);
void net_receive_frame(uint8_t* data, uint16_t len);
void net_send_packet(uint8_t* data, uint16_t len);

/* Interface functions */
net_interface_t* net_get_interface(const char* name);
void net_set_ipaddr(net_interface_t* iface, ipv4_addr_t addr);
void net_set_netmask(net_interface_t* iface, ipv4_addr_t mask);

/* Protocol functions */
void arp_init(void);
void arp_handle_packet(uint8_t* data, uint16_t len);
void arp_request(ipv4_addr_t target_ip);
mac_addr_t arp_lookup(ipv4_addr_t ip);

void ip_init(void);
void ip_handle_packet(uint8_t* data, uint16_t len);
void ip_send_packet(ipv4_addr_t dest, uint8_t proto, uint8_t* data, uint16_t len);

void icmp_init(void);
void icmp_handle_packet(uint8_t* data, uint16_t len);
void icmp_send_echo_request(ipv4_addr_t dest);

void udp_init(void);
void udp_handle_packet(uint8_t* data, uint16_t len);

void tcp_init(void);
void tcp_handle_packet(uint8_t* data, uint16_t len);

/* Utility functions */
uint16_t net_checksum(uint8_t* data, uint32_t len);
uint16_t net_htons(uint16_t value);
uint16_t net_ntohs(uint16_t value);
uint32_t net_htonl(uint32_t value);
uint32_t net_ntohl(uint32_t value);
void net_set_ipaddr_bytes(ipv4_addr_t* addr, uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void net_set_macaddr_bytes(mac_addr_t* addr, uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);

#endif
