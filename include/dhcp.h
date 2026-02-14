#ifndef DHCP_H
#define DHCP_H

#include "types.h"
#include "net.h"

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_BUFFER_SIZE 548

/* DHCP op codes */
#define DHCP_OP_BOOTREQUEST 1
#define DHCP_OP_BOOTREPLY   2

/* DHCP message types */
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8

/* DHCP packet (simplified) */
typedef struct {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    ipv4_addr_t ciaddr;
    ipv4_addr_t yiaddr;
    ipv4_addr_t siaddr;
    ipv4_addr_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312]; /* options area */
} __attribute__((packed)) dhcp_pkt_t;

/* DHCP server state */
typedef struct {
    int running;
    int sockfd;
    ipv4_addr_t pool_start; /* first assignable IP */
    ipv4_addr_t pool_end;   /* last assignable IP */
    ipv4_addr_t server_ip;  /* server IP (iface) */
    ipv4_addr_t netmask;
    ipv4_addr_t gateway;
} dhcp_server_t;

void dhcp_init(void);
void dhcp_start(void);
void dhcp_stop(void);
void dhcp_poll(void);

#endif
