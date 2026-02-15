#include "process.h"
#include "memory.h"
#include "string.h"
#include "drivers.h"

/* Global process table */
static process_t g_process_table[MAX_PROCESSES];
static uint32_t g_current_pid = 0;      /* Current process ID */
static uint8_t g_process_count = 0;     /* Number of active processes */
static uint32_t g_next_pid = 1;         /* Next PID to allocate */

/* Process stack area (shared by all processes) */
static uint8_t g_process_stacks[MAX_PROCESSES * PROCESS_STACK_SIZE];

/* Initialize process manager */
void process_init(void) {
    /* Initialize process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        g_process_table[i].pid = 0;
        g_process_table[i].state = PROC_STATE_UNUSED;
        g_process_table[i].priority = 128;  /* Default priority */
        g_process_table[i].exit_code = 0;
        g_process_table[i].name[0] = '\0';
    }

    /* Create kernel process (PID 0) */
    g_process_table[0].pid = 0;
    g_process_table[0].parent_pid = 0;
    g_process_table[0].state = PROC_STATE_RUNNING;
    g_process_table[0].priority = 0;  /* Highest priority for kernel */
    g_process_table[0].ticks = PROCESS_TIME_SLICE;
    strcpy(g_process_table[0].name, "kernel");
    g_process_table[0].stack_base = (uint32_t)g_process_stacks;
    g_process_table[0].stack_size = PROCESS_STACK_SIZE;

    g_current_pid = 0;
    g_process_count = 1;
    g_next_pid = 1;
}

/* Get current process */
process_t* process_current(void) {
    if (g_current_pid >= MAX_PROCESSES) {
        return NULL;
    }
    return &g_process_table[g_current_pid];
}

/* Get process by PID */
process_t* process_get(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    if (g_process_table[pid].state == PROC_STATE_UNUSED) {
        return NULL;
    }
    return &g_process_table[pid];
}

/* Get process by index */
process_t* process_get_at_index(uint8_t index) {
    if (index >= MAX_PROCESSES) {
        return NULL;
    }
    return &g_process_table[index];
}

/* Get number of active processes */
uint8_t process_count(void) {
    return g_process_count;
}

/* Allocate a PID */
static uint32_t process_allocate_pid(void) {
    uint32_t pid = g_next_pid;
    
    while (pid < MAX_PROCESSES) {
        if (g_process_table[pid].state == PROC_STATE_UNUSED) {
            g_next_pid = pid + 1;
            if (g_next_pid >= MAX_PROCESSES) {
                g_next_pid = 1;  /* Wrap around, skip kernel PID 0 */
            }
            return pid;
        }
        pid++;
    }

    /* Wrap around and try again */
    for (pid = 1; pid < g_next_pid; pid++) {
        if (g_process_table[pid].state == PROC_STATE_UNUSED) {
            return pid;
        }
    }

    return 0;  /* No PIDs available */
}

/* Create a new process */
int process_spawn(const char* name, void (*entry_point)(void), uint8_t priority) {
    uint32_t pid = process_allocate_pid();
    
    if (pid == 0 || pid >= MAX_PROCESSES) {
        return -1;  /* No available PIDs */
    }

    process_t* proc = &g_process_table[pid];

    /* Initialize process */
    proc->pid = pid;
    proc->parent_pid = g_current_pid;
    proc->state = PROC_STATE_READY;
    proc->priority = priority;
    proc->ticks = PROCESS_TIME_SLICE;
    proc->exit_code = 0;

    /* Set up stack */
    proc->stack_base = (uint32_t)g_process_stacks + (pid * PROCESS_STACK_SIZE);
    proc->stack_size = PROCESS_STACK_SIZE;
    uint32_t stack_top = proc->stack_base + PROCESS_STACK_SIZE - 4;

    /* Initialize context */
    proc->context.esp = stack_top;
    proc->context.eip = (uint32_t)entry_point;
    proc->context.ebp = stack_top;
    proc->context.eflags = 0x200;  /* IF flag set (interrupts enabled) */
    proc->context.eax = 0;
    proc->context.ebx = 0;
    proc->context.ecx = 0;
    proc->context.edx = 0;
    proc->context.esi = 0;
    proc->context.edi = 0;

    proc->entry_point = (uint32_t)entry_point;
    proc->created_ticks = pit_get_ticks();

    /* Copy name */
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = '\0';

    g_process_count++;
    return (int)pid;
}

/* Terminate current process */
void process_exit(int exit_code) {
    process_t* proc = process_current();
    if (!proc) {
        return;
    }

    proc->exit_code = exit_code;
    proc->state = PROC_STATE_TERMINATED;
    proc->terminated_ticks = pit_get_ticks();
    g_process_count--;

    /* Force context switch to next process */
    process_schedule();
}

/* Terminate a specific process */
int process_kill(uint32_t pid) {
    process_t* proc = process_get(pid);
    if (!proc) {
        return -1;
    }

    if (pid == 0) {
        return -1;  /* Cannot kill kernel */
    }

    proc->exit_code = -1;
    proc->state = PROC_STATE_TERMINATED;
    proc->terminated_ticks = pit_get_ticks();

    if (g_process_count > 0) {
        g_process_count--;
    }

    /* If we killed the current process, schedule next */
    if (g_current_pid == pid) {
        process_schedule();
    }

    return 0;
}

/* Round-robin scheduler with priority support */
process_t* process_find_next(void) {
    uint32_t start_pid = g_current_pid;
    uint32_t next_pid = (g_current_pid + 1) % MAX_PROCESSES;
    
    /* Find highest priority ready process */
    int best_pid = -1;
    int best_priority = 256;  /* Lower is better */

    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        uint32_t check_pid = (next_pid + i) % MAX_PROCESSES;
        process_t* proc = &g_process_table[check_pid];

        if (proc->state == PROC_STATE_READY) {
            if (proc->priority < best_priority) {
                best_priority = proc->priority;
                best_pid = check_pid;
                break;  /* Found higher priority, use it */
            }
        }
    }

    if (best_pid >= 0) {
        return &g_process_table[best_pid];
    }

    /* Fallback: return current process if still ready */
    if (g_process_table[g_current_pid].state == PROC_STATE_READY) {
        return &g_process_table[g_current_pid];
    }

    /* No ready process found, return kernel */
    return &g_process_table[0];
}

/* Schedule: Switch to next process */
void process_schedule(void) {
    process_t* next = process_find_next();
    if (!next) {
        next = &g_process_table[0];
    }

    /* Switch state: current -> READY, next -> RUNNING */
    process_t* current = process_current();
    if (current && current->pid != 0 && current->state == PROC_STATE_RUNNING) {
        current->state = PROC_STATE_READY;
    }

    g_current_pid = next->pid;
    next->state = PROC_STATE_RUNNING;
    next->ticks = PROCESS_TIME_SLICE;
}

/* Called from timer interrupt - decrement time slice */
void process_tick(void) {
    process_t* proc = process_current();
    if (!proc) {
        return;
    }

    if (proc->ticks > 0) {
        proc->ticks--;
    }

    /* Time slice expired */
    if (proc->ticks == 0) {
        process_schedule();
    }
}

/* Display all processes (for ps command) */
void process_display_info(void) {
    char buf[32];
    const char* states[] = {
        "unused",
        "ready",
        "running",
        "blocked",
        "terminated"
    };

    vga_write_string("PID  STATE      PRIORITY  NAME\n");
    vga_write_string("===  ========== ========  ============================\n");

    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t* proc = &g_process_table[i];

        if (proc->state == PROC_STATE_UNUSED) {
            continue;
        }

        /* PID */
        itoa(proc->pid, buf, 10);
        vga_write_string(buf);
        vga_write_string("  ");

        /* State */
        vga_write_string(states[proc->state]);
        for (int j = strlen(states[proc->state]); j < 9; j++) {
            vga_write_char(' ');
        }

        /* Priority */
        itoa(proc->priority, buf, 10);
        vga_write_string(buf);
        for (int j = strlen(buf); j < 9; j++) {
            vga_write_char(' ');
        }

        /* Name */
        vga_write_string(proc->name);
        vga_write_char('\n');
    }

    vga_write_string("\nTotal processes: ");
    itoa(g_process_count, buf, 10);
    vga_write_string(buf);
    vga_write_char('\n');
}
