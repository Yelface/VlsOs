#ifndef DNS_H
#define DNS_H

#include "types.h"
#include "net.h"

#define DNS_PORT 53
#define DNS_MAX_QNAME 256
#define DNS_BUFFER_SIZE 512

/* DNS header (network byte order) */
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} __attribute__((packed)) dns_hdr_t;

/* DNS question (variable length name) */

typedef struct {
    int running;
    int sockfd;
} dns_server_t;

void dns_init(void);
void dns_start(void);
void dns_stop(void);
void dns_poll(void);

#endif
