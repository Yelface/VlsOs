#ifndef SOCKET_H
#define SOCKET_H

#include "types.h"
#include "net.h"

/* Socket address family */
#define AF_INET     2

/* Socket types */
#define SOCK_STREAM 1  /* TCP */
#define SOCK_DGRAM  2  /* UDP */

/* Socket protocols */
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

/* Socket structure */
typedef struct {
	int domain;         /* AF_INET */
	int type;           /* SOCK_STREAM or SOCK_DGRAM */
	int protocol;       /* IPPROTO_TCP or IPPROTO_UDP */
	int fd;             /* File descriptor */
	ipv4_addr_t local_ip;
	uint16_t local_port;
	ipv4_addr_t remote_ip;
	uint16_t remote_port;
	int state;          /* Connection state */
	void* buffer;       /* Receive buffer */
	uint16_t buf_len;
} socket_t;

/* Socket states */
#define SOCK_CLOSED     0
#define SOCK_CREATED    1
#define SOCK_LISTENING  2
#define SOCK_CONNECTING 3
#define SOCK_CONNECTED  4

/* Socket API functions */
int socket(int domain, int type, int protocol);
int bind(int sockfd, ipv4_addr_t addr, uint16_t port);
int listen(int sockfd, int backlog);
int accept(int sockfd, ipv4_addr_t* addr, uint16_t* port);
int connect(int sockfd, ipv4_addr_t addr, uint16_t port);
int send(int sockfd, uint8_t* buffer, uint16_t len);
int recv(int sockfd, uint8_t* buffer, uint16_t maxlen);
int sendto(int sockfd, uint8_t* buffer, uint16_t len, ipv4_addr_t addr, uint16_t port);
int recvfrom(int sockfd, uint8_t* buffer, uint16_t maxlen, ipv4_addr_t* addr, uint16_t* port);
int close(int sockfd);

#endif
