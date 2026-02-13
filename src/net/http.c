#include "http.h"
#include "socket.h"
#include "string.h"
#include "drivers.h"
#include "memory.h"

static http_server_t http_server;

void http_server_init(void) {
	memset(&http_server, 0, sizeof(http_server_t));
	http_server.server_socket = -1;
	http_server.running = 0;
	http_server.requests_served = 0;
}

void http_server_start(void) {
	if (http_server.running) {
		vga_write_string("HTTP server already running\n");
		return;
	}

	/* Create TCP socket */
	http_server.server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (http_server.server_socket < 0) {
		vga_write_string("Failed to create HTTP server socket\n");
		return;
	}

	/* Bind to port 80 */
	ipv4_addr_t any_addr;
	memset(&any_addr, 0, sizeof(ipv4_addr_t));
	
	if (bind(http_server.server_socket, any_addr, HTTP_PORT) < 0) {
		vga_write_string("Failed to bind HTTP server to port 80\n");
		close(http_server.server_socket);
		return;
	}

	/* Listen for connections */
	if (listen(http_server.server_socket, HTTP_MAX_CLIENTS) < 0) {
		vga_write_string("Failed to listen on HTTP server socket\n");
		close(http_server.server_socket);
		return;
	}

	http_server.running = 1;
	vga_write_string("HTTP server started on port 80\n");
}

void http_server_stop(void) {
	if (!http_server.running) {
		vga_write_string("HTTP server not running\n");
		return;
	}

	close(http_server.server_socket);
	http_server.running = 0;
	vga_write_string("HTTP server stopped\n");
}

void http_server_poll(void) {
	if (!http_server.running) {
		return;
	}

	/* Accept new connections */
	for (int i = 0; i < HTTP_MAX_CLIENTS; i++) {
		if (!http_server.clients[i].active) {
			ipv4_addr_t client_addr;
			uint16_t client_port;
			int client_sock = accept(http_server.server_socket, &client_addr, &client_port);
			
			if (client_sock >= 0) {
				http_server.clients[i].active = 1;
				http_server.clients[i].client_ip = *(uint32_t*)&client_addr;
				http_server.clients[i].client_port = client_port;
				http_server.clients[i].buffer_len = 0;
			}
		}
	}

	/* Handle existing clients */
	for (int i = 0; i < HTTP_MAX_CLIENTS; i++) {
		if (http_server.clients[i].active) {
			http_handle_client(&http_server.clients[i]);
		}
	}
}

void http_handle_client(http_client_t* client) {
	/* For now, send a simple HTML response */
	const char* html_response = 
		"<html><head><title>VlsOs</title></head>"
		"<body><h1>Welcome to VlsOs HTTP Server!</h1>"
		"<p>This is a simple web server running inside an x86 OS.</p>"
		"<p>Requests served: ";
	
	http_send_response(client, HTTP_200_OK, html_response);
	client->active = 0;
}

void http_send_response(http_client_t* client, uint16_t status_code, const char* body) {
	char response[512];
	char status_text[32];
	char count_str[16];

	/* Get status text */
	switch (status_code) {
		case HTTP_200_OK:
			strcpy(status_text, "OK");
			break;
		case HTTP_400_BAD_REQUEST:
			strcpy(status_text, "Bad Request");
			break;
		case HTTP_404_NOT_FOUND:
			strcpy(status_text, "Not Found");
			break;
		case HTTP_500_INTERNAL_SERVER_ERROR:
			strcpy(status_text, "Internal Server Error");
			break;
		default:
			strcpy(status_text, "Unknown");
	}

	/* Build response */
	strcpy(response, "HTTP/1.1 ");
	itoa(status_code, count_str, 10);
	strcat(response, count_str);
	strcat(response, " ");
	strcat(response, status_text);
	strcat(response, "\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
	strcat(response, body);
	
	itoa(http_server.requests_served, count_str, 10);
	strcat(response, count_str);
	strcat(response, "</p></body></html>");

	/* Send response (stub - full implementation would use socket send) */
	vga_write_string("HTTP/1.1 ");
	itoa(status_code, count_str, 10);
	vga_write_string(count_str);
	vga_write_string("\n");

	http_server.requests_served++;
}
