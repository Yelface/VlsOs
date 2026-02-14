#include "dhcp.h"
#include "socket.h"
#include "memory.h"
#include "string.h"
#include "drivers.h"
#include "net.h"

static dhcp_server_t dhcp_server;

static void dhcp_fill_offer(dhcp_pkt_t* req, dhcp_pkt_t* resp, ipv4_addr_t offer_ip) {
    memset(resp, 0, sizeof(dhcp_pkt_t));
    resp->op = DHCP_OP_BOOTREPLY;
    resp->htype = req->htype;
    resp->hlen = req->hlen;
    resp->xid = req->xid;
    memcpy(resp->chaddr, req->chaddr, 16);
    resp->yiaddr = offer_ip;
    resp->siaddr = dhcp_server.server_ip;
    /* minimal options: magic cookie + DHCP message type + subnet + router */
    uint8_t* opt = resp->options;
    /* magic cookie */
    opt[0] = 99; opt[1] = 130; opt[2] = 83; opt[3] = 99; 
    int p = 4;
    /* DHCP Message Type */
    opt[p++] = 53; opt[p++] = 1; opt[p++] = DHCPOFFER;
    /* Subnet Mask (option 1)
       encode 4 bytes */
    opt[p++] = 1; opt[p++] = 4;
    memcpy(&opt[p], &dhcp_server.netmask.octets, 4); p += 4;
    /* Router (option 3) */
    opt[p++] = 3; opt[p++] = 4; memcpy(&opt[p], &dhcp_server.gateway.octets,4); p+=4;
    /* Server Identifier (option 54) */
    opt[p++] = 54; opt[p++] = 4; memcpy(&opt[p], &dhcp_server.server_ip.octets,4); p+=4;
    /* Lease time (option 51) 3600s */
    opt[p++] = 51; opt[p++] = 4; opt[p++]=0; opt[p++]=0; opt[p++]=0x0E; opt[p++]=0x10; /* 3600 decimal = 0x0E10 */
    /* End option */
    opt[p++] = 255;
}

static void dhcp_fill_ack(dhcp_pkt_t* req, dhcp_pkt_t* resp, ipv4_addr_t assign_ip) {
    dhcp_fill_offer(req, resp, assign_ip);
    /* change message type to ACK */
    /* find option 53 and set to ACK */
    uint8_t* opt = resp->options;
    for (int i=4; i < 312; ) {
        uint8_t code = opt[i];
        if (code == 255) break;
        if (i+1 >= 312) break;
        uint8_t len = opt[i+1];
        if (code == 53 && len >=1) { opt[i+2] = DHCPACK; break; }
        i += 2 + len;
    }
}

void dhcp_init(void) {
    memset(&dhcp_server, 0, sizeof(dhcp_server_t));
    dhcp_server.running = 0;
    dhcp_server.sockfd = -1;
    /* default pool: 192.168.1.150 - 192.168.1.160 */
    net_set_ipaddr_bytes(&dhcp_server.pool_start, 192,168,1,150);
    net_set_ipaddr_bytes(&dhcp_server.pool_end, 192,168,1,160);
    net_interface_t* iface = net_get_interface("eth0");
    if (iface) {
        dhcp_server.server_ip = iface->ip_addr;
        dhcp_server.netmask = iface->netmask;
        dhcp_server.gateway = iface->gateway;
    }
}

void dhcp_start(void) {
    if (dhcp_server.running) { vga_write_string("DHCP server already running\n"); return; }
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) { vga_write_string("Failed to create DHCP socket\n"); return; }
    ipv4_addr_t any; memset(&any,0,sizeof(any));
    if (bind(s, any, DHCP_SERVER_PORT) < 0) { vga_write_string("Failed to bind DHCP socket\n"); close(s); return; }
    dhcp_server.sockfd = s;
    dhcp_server.running = 1;
    vga_write_string("DHCP server started on port 67\n");
}

void dhcp_stop(void) {
    if (!dhcp_server.running) { vga_write_string("DHCP server not running\n"); return; }
    close(dhcp_server.sockfd);
    dhcp_server.sockfd = -1;
    dhcp_server.running = 0;
    vga_write_string("DHCP server stopped\n");
}

/* Very small DHCP poll: respond to DISCOVER with OFFER and REQUEST with ACK
   No stateful lease database; always offers from pool_start */
void dhcp_poll(void) {
    if (!dhcp_server.running) return;
    uint8_t buf[DHCP_BUFFER_SIZE];
    ipv4_addr_t from; uint16_t port=0;
    int n = recvfrom(dhcp_server.sockfd, buf, sizeof(buf), &from, &port);
    if (n <= 0) return;
    if (n < (int)sizeof(dhcp_pkt_t) - 312) return; /* minimal size */
    dhcp_pkt_t* req = (dhcp_pkt_t*)buf;
    /* find DHCP message type option (53) in options area */
    int typ = 0;
    uint8_t* opt = req->options;
    for (int i=0; i<312; ) {
        uint8_t code = opt[i];
        if (code == 255) break;
        if (i+1 >= 312) break;
        uint8_t len = opt[i+1];
        if (code == 53 && len >= 1) { typ = opt[i+2]; break; }
        i += 2 + len;
    }
    /* choose offered IP = pool_start (simple) */
    ipv4_addr_t offer_ip = dhcp_server.pool_start;
    dhcp_pkt_t resp;
    if (typ == DHCPDISCOVER) {
        dhcp_fill_offer(req, &resp, offer_ip);
        /* send to broadcast 255.255.255.255 on client port */
        ipv4_addr_t b; net_set_ipaddr_bytes(&b,255,255,255,255);
        sendto(dhcp_server.sockfd, (uint8_t*)&resp, sizeof(dhcp_pkt_t), b, DHCP_CLIENT_PORT);
        vga_write_string("DHCP: Sent DHCPOFFER\n");
    } else if (typ == DHCPREQUEST) {
        dhcp_fill_ack(req, &resp, offer_ip);
        ipv4_addr_t b; net_set_ipaddr_bytes(&b,255,255,255,255);
        sendto(dhcp_server.sockfd, (uint8_t*)&resp, sizeof(dhcp_pkt_t), b, DHCP_CLIENT_PORT);
        vga_write_string("DHCP: Sent DHCPACK\n");
    }
}
