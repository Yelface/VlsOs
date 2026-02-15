#include "ipc.h"
#include "memory.h"
#include "string.h"
#include "process.h"

/* IPC Implementation - Pipes and Message Queues */

/* Global pipe table */
static ipc_pipe_t g_pipes[IPC_MAX_PIPES];
static ipc_queue_t g_queues[IPC_MAX_PIPES];

/* Initialize IPC subsystem */
void ipc_init(void) {
    for (int i = 0; i < IPC_MAX_PIPES; i++) {
        g_pipes[i].in_use = 0;
        g_pipes[i].read_pos = 0;
        g_pipes[i].write_pos = 0;
        g_pipes[i].message_count = 0;
        
        g_queues[i].in_use = 0;
        g_queues[i].buffer = NULL;
        g_queues[i].front = 0;
        g_queues[i].rear = 0;
        g_queues[i].size = 0;
        g_queues[i].max_size = 0;
    }
}

/* Create a pipe */
int ipc_pipe_create(void) {
    int pipe_id = -1;
    
    /* Find free pipe slot */
    for (int i = 0; i < IPC_MAX_PIPES; i++) {
        if (!g_pipes[i].in_use) {
            pipe_id = i;
            break;
        }
    }

    if (pipe_id < 0) {
        return -1;  /* No free pipes */
    }

    /* Initialize pipe */
    g_pipes[pipe_id].in_use = 1;
    g_pipes[pipe_id].read_pos = 0;
    g_pipes[pipe_id].write_pos = 0;
    g_pipes[pipe_id].message_count = 0;
    
    process_t* proc = process_current();
    g_pipes[pipe_id].owner_pid = proc ? proc->pid : 0;

    return pipe_id;
}

/* Close a pipe */
int ipc_pipe_close(int pipe_id) {
    if (pipe_id < 0 || pipe_id >= IPC_MAX_PIPES) {
        return -1;
    }

    if (!g_pipes[pipe_id].in_use) {
        return -1;
    }

    g_pipes[pipe_id].in_use = 0;
    return 0;
}

/* Write to pipe */
int ipc_pipe_write(int pipe_id, uint32_t message) {
    if (pipe_id < 0 || pipe_id >= IPC_MAX_PIPES) {
        return -1;
    }

    if (!g_pipes[pipe_id].in_use) {
        return -1;
    }

    ipc_pipe_t* pipe = &g_pipes[pipe_id];

    /* Check if buffer is full */
    if (pipe->message_count >= IPC_BUFFER_SIZE) {
        return -1;  /* Buffer full, can't write */
    }

    /* Write message */
    pipe->buffer[pipe->write_pos] = message;
    pipe->write_pos = (pipe->write_pos + 1) % IPC_BUFFER_SIZE;
    pipe->message_count++;

    return 0;
}

/* Read from pipe */
int ipc_pipe_read(int pipe_id, uint32_t* message) {
    if (pipe_id < 0 || pipe_id >= IPC_MAX_PIPES || !message) {
        return -1;
    }

    if (!g_pipes[pipe_id].in_use) {
        return -1;
    }

    ipc_pipe_t* pipe = &g_pipes[pipe_id];

    /* Check if buffer is empty */
    if (pipe->message_count == 0) {
        return -1;  /* No data */
    }

    /* Read message */
    *message = pipe->buffer[pipe->read_pos];
    pipe->read_pos = (pipe->read_pos + 1) % IPC_BUFFER_SIZE;
    pipe->message_count--;

    return 0;
}

/* Check if pipe has data */
int ipc_pipe_has_data(int pipe_id) {
    if (pipe_id < 0 || pipe_id >= IPC_MAX_PIPES) {
        return 0;
    }

    if (!g_pipes[pipe_id].in_use) {
        return 0;
    }

    return g_pipes[pipe_id].message_count > 0 ? 1 : 0;
}

/* Create message queue */
int ipc_queue_create(uint32_t size) {
    int queue_id = -1;
    
    /* Find free queue slot */
    for (int i = 0; i < IPC_MAX_PIPES; i++) {
        if (!g_queues[i].in_use) {
            queue_id = i;
            break;
        }
    }

    if (queue_id < 0) {
        return -1;  /* No free queues */
    }

    /* Allocate buffer */
    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) {
        return -1;
    }

    /* Initialize queue */
    g_queues[queue_id].in_use = 1;
    g_queues[queue_id].buffer = buffer;
    g_queues[queue_id].front = 0;
    g_queues[queue_id].rear = 0;
    g_queues[queue_id].size = 0;
    g_queues[queue_id].max_size = size;

    return queue_id;
}

/* Destroy message queue */
int ipc_queue_destroy(int queue_id) {
    if (queue_id < 0 || queue_id >= IPC_MAX_PIPES) {
        return -1;
    }

    if (!g_queues[queue_id].in_use) {
        return -1;
    }

    if (g_queues[queue_id].buffer) {
        free(g_queues[queue_id].buffer);
    }

    g_queues[queue_id].in_use = 0;
    return 0;
}

/* Send message to queue */
int ipc_queue_send(int queue_id, const void* data, uint32_t data_size) {
    if (queue_id < 0 || queue_id >= IPC_MAX_PIPES || !data) {
        return -1;
    }

    if (!g_queues[queue_id].in_use) {
        return -1;
    }

    ipc_queue_t* queue = &g_queues[queue_id];

    /* Check if buffer has enough space */
    if (queue->size + data_size > queue->max_size) {
        return -1;  /* No space */
    }

    /* Copy data to queue */
    uint32_t space_at_end = queue->max_size - queue->rear;
    
    if (data_size <= space_at_end) {
        memcpy(queue->buffer + queue->rear, data, data_size);
        queue->rear = (queue->rear + data_size) % queue->max_size;
    } else {
        /* Wrap around */
        memcpy(queue->buffer + queue->rear, data, space_at_end);
        memcpy(queue->buffer, (uint8_t*)data + space_at_end, data_size - space_at_end);
        queue->rear = data_size - space_at_end;
    }

    queue->size += data_size;
    return 0;
}

/* Receive message from queue */
int ipc_queue_recv(int queue_id, void* data, uint32_t max_size) {
    if (queue_id < 0 || queue_id >= IPC_MAX_PIPES || !data) {
        return -1;
    }

    if (!g_queues[queue_id].in_use) {
        return -1;
    }

    ipc_queue_t* queue = &g_queues[queue_id];

    /* Check if queue is empty */
    if (queue->size == 0) {
        return 0;  /* No data */
    }

    /* Determine read size */
    uint32_t read_size = max_size < queue->size ? max_size : queue->size;

    /* Copy data from queue */
    uint32_t space_at_end = queue->max_size - queue->front;
    
    if (read_size <= space_at_end) {
        memcpy(data, queue->buffer + queue->front, read_size);
        queue->front = (queue->front + read_size) % queue->max_size;
    } else {
        /* Wrap around */
        memcpy(data, queue->buffer + queue->front, space_at_end);
        memcpy((uint8_t*)data + space_at_end, queue->buffer, read_size - space_at_end);
        queue->front = read_size - space_at_end;
    }

    queue->size -= read_size;
    return (int)read_size;
}

/* Check queue size */
uint32_t ipc_queue_size(int queue_id) {
    if (queue_id < 0 || queue_id >= IPC_MAX_PIPES) {
        return 0;
    }

    if (!g_queues[queue_id].in_use) {
        return 0;
    }

    return g_queues[queue_id].size;
}
