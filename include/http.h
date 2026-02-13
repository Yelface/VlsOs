#ifndef HTTP_H
#define HTTP_H

#include "types.h"
#include "net.h"

/* HTTP Server Configuration */
#define HTTP_PORT           80
#define HTTP_MAX_CLIENTS    4
#define HTTP_BUFFER_SIZE    1024
#define HTTP_MAX_PATH_LEN   256

/* HTTP Methods */
#define HTTP_METHOD_GET     1
#define HTTP_METHOD_POST    2
#define HTTP_METHOD_HEAD    3

/* HTTP Response Codes */
#define HTTP_200_OK                    200
#define HTTP_400_BAD_REQUEST           400
#define HTTP_404_NOT_FOUND             404
#define HTTP_500_INTERNAL_SERVER_ERROR 500

/* HTTP Client Connection State */
typedef struct {
	uint32_t client_ip;
	uint16_t client_port;
	uint8_t method;
	char path[HTTP_MAX_PATH_LEN];
	uint8_t buffer[HTTP_BUFFER_SIZE];
	uint16_t buffer_len;
	int active;
} http_client_t;

/* HTTP Server State */
typedef struct {
	int server_socket;
	int running;
	http_client_t clients[HTTP_MAX_CLIENTS];
	uint32_t requests_served;
} http_server_t;

/* HTTP Functions */
void http_server_init(void);
void http_server_start(void);
void http_server_stop(void);
void http_server_poll(void);
void http_handle_client(http_client_t* client);
void http_send_response(http_client_t* client, uint16_t status_code, const char* body);

#endif
