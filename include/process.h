#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* Process management header */

/* Process states */
typedef enum {
    PROC_STATE_UNUSED = 0,
    PROC_STATE_READY = 1,
    PROC_STATE_RUNNING = 2,
    PROC_STATE_BLOCKED = 3,
    PROC_STATE_TERMINATED = 4
} process_state_t;

/* CPU context (register state for context switching) */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
} cpu_context_t;

/* Process structure */
typedef struct {
    uint32_t pid;               /* Process ID (1-31, 0 reserved for kernel) */
    uint32_t parent_pid;        /* Parent process ID */
    process_state_t state;      /* Current state */
    uint8_t priority;           /* Priority (0=highest, 255=lowest) */
    uint8_t ticks;              /* Remaining time slice */
    
    uint32_t stack_base;        /* Stack base address */
    uint32_t stack_size;        /* Stack size (in bytes) */
    
    cpu_context_t context;      /* CPU context at last switch */
    
    uint32_t entry_point;       /* Entry point for new processes */
    int exit_code;              /* Exit code */
    
    char name[32];              /* Process name */
    
    uint32_t created_ticks;     /* Ticks when created */
    uint32_t terminated_ticks;  /* Ticks when terminated */
} process_t;

#define MAX_PROCESSES 32
#define PROCESS_STACK_SIZE 4096  /* 4KB stack per process */
#define PROCESS_TIME_SLICE 10    /* Timer ticks per time slice */

/* Process manager functions */

/* Initialize process manager */
void process_init(void);

/* Get current process */
process_t* process_current(void);

/* Create a new process (spawn) */
int process_spawn(const char* name, void (*entry_point)(void), uint8_t priority);

/* Terminate current process with exit code */
void process_exit(int exit_code);

/* Terminate a specific process */
int process_kill(uint32_t pid);

/* Schedule next process (called by timer interrupt) */
void process_schedule(void);

/* Get process info by PID */
process_t* process_get(uint32_t pid);

/* Get number of active processes */
uint8_t process_count(void);

/* Get process by index (for iteration) */
process_t* process_get_at_index(uint8_t index);

/* Display process information (for ps command) */
void process_display_info(void);

/* Switch to next process (internal, called from context switch) */
void process_do_switch(cpu_context_t* current_context);

#endif
