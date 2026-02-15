#ifndef IPC_H
#define IPC_H

#include "types.h"

/* Inter-Process Communication - Pipes and Message Queues */

/* Pipe structure */
typedef struct {
    uint32_t buffer[256];   /* Message buffer */
    uint16_t read_pos;      /* Read position */
    uint16_t write_pos;     /* Write position */
    uint32_t message_count; /* Number of messages */
    uint8_t in_use;         /* Is this pipe active */
    uint32_t owner_pid;     /* PID of process that created the pipe */
} ipc_pipe_t;

#define IPC_MAX_PIPES       32
#define IPC_BUFFER_SIZE     256
#define IPC_MESSAGE_SIZE    4   /* Messages are 32-bit values */

/* Message queue structure */
typedef struct {
    uint8_t* buffer;
    uint32_t front;
    uint32_t rear;
    uint32_t size;
    uint32_t max_size;
    uint8_t in_use;
} ipc_queue_t;

/* Create a pipe */
int ipc_pipe_create(void);

/* Close a pipe */
int ipc_pipe_close(int pipe_id);

/* Write to pipe */
int ipc_pipe_write(int pipe_id, uint32_t message);

/* Read from pipe */
int ipc_pipe_read(int pipe_id, uint32_t* message);

/* Check if pipe has data */
int ipc_pipe_has_data(int pipe_id);

/* Create message queue */
int ipc_queue_create(uint32_t size);

/* Destroy message queue */
int ipc_queue_destroy(int queue_id);

/* Send message to queue */
int ipc_queue_send(int queue_id, const void* data, uint32_t size);

/* Receive message from queue */
int ipc_queue_recv(int queue_id, void* data, uint32_t max_size);

/* Check queue size */
uint32_t ipc_queue_size(int queue_id);

/* Initialize IPC subsystem */
void ipc_init(void);

#endif
