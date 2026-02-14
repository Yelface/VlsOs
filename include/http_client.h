#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "types.h"
#include "net.h"

#define HTTP_CLIENT_BUFFER_SIZE 2048

/* HTTP response status */
typedef struct {
	uint16_t status_code;
	uint32_t content_length;
	uint8_t* body;
	uint16_t body_len;
} http_response_t;

/* HTTP client functions */
int http_client_get(const char* host, uint16_t port, const char* path, http_response_t* resp);
void http_client_free(http_response_t* resp);

#endif
