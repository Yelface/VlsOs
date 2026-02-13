#include "socket.h"
#include "memory.h"

#define MAX_SOCKETS 16

static socket_t sockets[MAX_SOCKETS];
static int next_fd = 1;

int socket(int domain, int type, int protocol) {
	if (domain != AF_INET || (type != SOCK_STREAM && type != SOCK_DGRAM)) {
		return -1;
	}

	/* Find free socket */
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].state == SOCK_CLOSED) {
			sockets[i].domain = domain;
			sockets[i].type = type;
			sockets[i].protocol = protocol;
			sockets[i].fd = next_fd++;
			sockets[i].state = SOCK_CREATED;
			sockets[i].local_port = 0;
			sockets[i].buffer = malloc(NET_MTU);
			sockets[i].buf_len = 0;
			return sockets[i].fd;
		}
	}

	return -1;  /* No free sockets */
}

int bind(int sockfd, ipv4_addr_t addr, uint16_t port) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state != SOCK_CLOSED) {
			sockets[i].local_ip = addr;
			sockets[i].local_port = port;
			return 0;
		}
	}
	return -1;
}

int listen(int sockfd, int backlog) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state == SOCK_CREATED) {
			sockets[i].state = SOCK_LISTENING;
			return 0;
		}
	}
	return -1;
}

int accept(int sockfd, ipv4_addr_t* addr, uint16_t* port) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state == SOCK_LISTENING) {
			/* Would wait for incoming connection */
			return -1;  /* Not yet implemented */
		}
	}
	return -1;
}

int connect(int sockfd, ipv4_addr_t addr, uint16_t port) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state == SOCK_CREATED) {
			sockets[i].remote_ip = addr;
			sockets[i].remote_port = port;
			sockets[i].state = SOCK_CONNECTING;
			/* Send SYN for TCP or just mark connected for UDP */
			if (sockets[i].type == SOCK_DGRAM) {
				sockets[i].state = SOCK_CONNECTED;
			}
			return 0;
		}
	}
	return -1;
}

int send(int sockfd, uint8_t* buffer, uint16_t len) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state == SOCK_CONNECTED) {
			if (sockets[i].type == SOCK_DGRAM) {
				return sendto(sockfd, buffer, len, sockets[i].remote_ip, sockets[i].remote_port);
			} else if (sockets[i].type == SOCK_STREAM) {
				/* Would send TCP data */
				return len;
			}
		}
	}
	return -1;
}

int recv(int sockfd, uint8_t* buffer, uint16_t maxlen) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state == SOCK_CONNECTED) {
			/* Would receive data */
			return sockets[i].buf_len;
		}
	}
	return -1;
}

int sendto(int sockfd, uint8_t* buffer, uint16_t len, ipv4_addr_t addr, uint16_t port) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd) {
			if (sockets[i].type == SOCK_DGRAM) {
				udp_send_packet(addr, sockets[i].local_port, port, buffer, len);
				return len;
			}
		}
	}
	return -1;
}

int recvfrom(int sockfd, uint8_t* buffer, uint16_t maxlen, ipv4_addr_t* addr, uint16_t* port) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd) {
			/* Would receive UDP packet */
			return sockets[i].buf_len;
		}
	}
	return -1;
}

int close(int sockfd) {
	for (int i = 0; i < MAX_SOCKETS; i++) {
		if (sockets[i].fd == sockfd && sockets[i].state != SOCK_CLOSED) {
			if (sockets[i].buffer) {
				free(sockets[i].buffer);
			}
			sockets[i].state = SOCK_CLOSED;
			return 0;
		}
	}
	return -1;
}
