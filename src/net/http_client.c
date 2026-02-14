#include "http_client.h"
#include "socket.h"
#include "string.h"
#include "memory.h"
#include "drivers.h"

/* Simple HTTP client: GET request and response parsing */

static int http_client_parse_response(uint8_t* buf, uint16_t buflen, http_response_t* resp) {
	/* Parse HTTP response: "HTTP/1.x NNN ..." */
	if (buflen < 12) return -1;
	
	/* Skip to first space, read status code */
	int i = 0;
	while (i < buflen && buf[i] != ' ') i++;
	if (i >= buflen) return -1;
	i++; /* skip space */
	
	/* Read 3-digit status code */
	char code_str[4] = {0};
	if (i + 3 > buflen) return -1;
	code_str[0] = buf[i]; code_str[1] = buf[i+1]; code_str[2] = buf[i+2]; code_str[3] = 0;
	resp->status_code = atoi(code_str);
	i += 3;
	
	/* Skip to \r\n */
	while (i < buflen && buf[i] != '\n') i++;
	if (i >= buflen) return -1;
	i++; /* skip \n */
	
	/* Parse headers looking for Content-Length */
	resp->content_length = 0;
	while (i < buflen && buf[i] != '\n') {
		/* Check for "Content-Length: " */
		if (i + 16 <= buflen && memcmp(&buf[i], "Content-Length: ", 16) == 0) {
			i += 16;
			char len_str[16] = {0};
			int j = 0;
			while (i < buflen && j < 15 && buf[i] >= '0' && buf[i] <= '9') {
				len_str[j++] = buf[i++];
			}
			len_str[j] = 0;
			resp->content_length = atoi(len_str);
		}
		
		/* Skip to next line */
		while (i < buflen && buf[i] != '\n') i++;
		if (i < buflen) i++; /* skip \n */
	}
	
	if (i < buflen) i++; /* skip final \n before body */
	
	/* Body starts here */
	resp->body = malloc(buflen - i + 1);
	if (!resp->body) return -1;
	resp->body_len = buflen - i;
	memcpy(resp->body, &buf[i], resp->body_len);
	resp->body[resp->body_len] = 0;
	
	return 0;
}

int http_client_get(const char* host, uint16_t port, const char* path, http_response_t* resp) {
	memset(resp, 0, sizeof(http_response_t));
	
	/* Parse IP from host (simple dotted-quad) */
	ipv4_addr_t server_ip;
	int octets[4] = {0};
	int octet_count = 0;
	char num_buf[4] = {0};
	int num_idx = 0;
	
	for (int i = 0; host[i] && octet_count < 4; i++) {
		if (host[i] == '.' || host[i] == 0) {
			if (num_idx > 0) {
				octets[octet_count++] = atoi(num_buf);
				num_idx = 0;
				num_buf[0] = 0;
			}
		} else if (host[i] >= '0' && host[i] <= '9' && num_idx < 3) {
			num_buf[num_idx++] = host[i];
			num_buf[num_idx] = 0;
		}
	}
	
	if (octet_count < 4) {
		octets[octet_count++] = atoi(num_buf);
	}
	
	if (octet_count != 4) {
		vga_write_string("Invalid host IP\n");
		return -1;
	}
	
	server_ip.octets[0] = octets[0];
	server_ip.octets[1] = octets[1];
	server_ip.octets[2] = octets[2];
	server_ip.octets[3] = octets[3];
	
	/* Create TCP socket */
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		vga_write_string("Failed to create socket\n");
		return -1;
	}
	
	/* Connect to server */
	if (connect(sock, server_ip, port) < 0) {
		vga_write_string("Failed to connect\n");
		close(sock);
		return -1;
	}
	
	/* Build and send HTTP request */
	uint8_t request[512];
	int req_len = 0;
	
	/* "GET /path HTTP/1.1\r\n" */
	strcpy((char*)request, "GET ");
	req_len = 4;
	strcpy((char*)&request[req_len], path ? path : "/");
	req_len += strlen(path ? path : "/");
	strcpy((char*)&request[req_len], " HTTP/1.1\r\n");
	req_len += 11;
	
	/* "Host: host\r\n" */
	strcpy((char*)&request[req_len], "Host: ");
	req_len += 6;
	strcpy((char*)&request[req_len], host);
	req_len += strlen(host);
	strcpy((char*)&request[req_len], "\r\n\r\n");
	req_len += 4;
	
	if (send(sock, request, req_len) < 0) {
		vga_write_string("Failed to send request\n");
		close(sock);
		return -1;
	}
	
	/* Receive response */
	uint8_t response_buf[HTTP_CLIENT_BUFFER_SIZE];
	int recv_len = recv(sock, response_buf, sizeof(response_buf));
	close(sock);
	
	if (recv_len <= 0) {
		vga_write_string("Failed to receive response\n");
		return -1;
	}
	
	/* Parse response */
	if (http_client_parse_response(response_buf, recv_len, resp) < 0) {
		vga_write_string("Failed to parse response\n");
		return -1;
	}
	
	return 0;
}

void http_client_free(http_response_t* resp) {
	if (resp->body) {
		free(resp->body);
		resp->body = NULL;
	}
	resp->body_len = 0;
}
