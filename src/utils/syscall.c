/* VlsOs Syscall Implementation with Filesystem Support */

#include "types.h"
#include "syscall.h"
#include "filesystem.h"
#include "process.h"
#include "drivers.h"
#include "shell.h"

/* Global file descriptor table for kernel */
#define KERNEL_MAX_FILES 16
static int g_kernel_fds[KERNEL_MAX_FILES] = {-1};

/* Standard I/O streams (reserved) */
#define STDIN_FD   0
#define STDOUT_FD  1
#define STDERR_FD  2

/* System call handler dispatcher */
void syscall_handler(syscall_context_t* ctx) {
    uint32_t syscall_num = ctx->eax;
    int32_t result = -1;

    switch (syscall_num) {
        case SYS_EXIT:
            sys_exit((int)ctx->ebx);
            break;

        case SYS_WRITE:
            result = sys_write((int)ctx->ebx, (void*)ctx->ecx, ctx->edx);
            break;

        case SYS_READ:
            result = sys_read((int)ctx->ebx, (void*)ctx->ecx, ctx->edx);
            break;

        case SYS_OPEN:
            result = sys_open((const char*)ctx->ebx, ctx->ecx);
            break;

        case SYS_CLOSE:
            result = sys_close((int)ctx->ebx);
            break;

        case SYS_GETPID:
            result = (int32_t)sys_getpid();
            break;

        case SYS_KILL:
            result = sys_kill(ctx->ebx, (int)ctx->ecx);
            break;

        case SYS_PIPE:
            result = sys_pipe((int*)ctx->ebx);
            break;

        case SYS_SEEK:
            result = sys_seek((int)ctx->ebx, ctx->ecx);
            break;

        default:
            result = -1;  /* Unknown syscall */
            break;
    }

    ctx->eax = (uint32_t)result;
}

/* Exit process */
void sys_exit(int code) {
    process_exit(code);
}

/* Write to file descriptor */
int sys_write(int fd, const void* buffer, uint32_t count) {
    if (!buffer || count == 0) {
        return -1;
    }

    /* Handle standard streams */
    if (fd == STDOUT_FD || fd == STDERR_FD) {
        /* Write to VGA display */
        const char* str = (const char*)buffer;
        for (uint32_t i = 0; i < count && str[i]; i++) {
            vga_write_char(str[i]);
        }
        return (int)count;
    }

    if (fd == STDIN_FD) {
        return -1;  /* Can't write to stdin */
    }

    /* File operations */
    if (fd >= FIRST_USER_FD && fd < (FIRST_USER_FD + KERNEL_MAX_FILES)) {
        int real_fd = g_kernel_fds[fd - FIRST_USER_FD];
        if (real_fd >= 0) {
            return fs_write(real_fd, (const uint8_t*)buffer, (uint16_t)count);
        }
    }

    return -1;
}

/* Read from file descriptor */
int sys_read(int fd, void* buffer, uint32_t count) {
    if (!buffer || count == 0) {
        return -1;
    }

    if (fd == STDIN_FD) {
        /* Read from keyboard */
        char* str = (char*)buffer;
        uint32_t i = 0;
        while (i < count) {
            char c = keyboard_read_char();
            if (c == '\n') {
                str[i] = 0;
                return (int)i;
            }
            str[i++] = c;
        }
        return (int)count;
    }

    if (fd >= FIRST_USER_FD && fd < (FIRST_USER_FD + KERNEL_MAX_FILES)) {
        int real_fd = g_kernel_fds[fd - FIRST_USER_FD];
        if (real_fd >= 0) {
            return fs_read(real_fd, (uint8_t*)buffer, (uint16_t)count);
        }
    }

    return -1;
}

/* Open file */
int sys_open(const char* filename, uint32_t flags) {
    if (!filename) {
        return -1;
    }

    int real_fd = fs_open(filename, 0);
    if (real_fd < 0) {
        return -1;
    }

    /* Find free user-mode file descriptor slot */
    for (int i = 0; i < KERNEL_MAX_FILES; i++) {
        if (g_kernel_fds[i] == -1) {
            g_kernel_fds[i] = real_fd;
            return FIRST_USER_FD + i;
        }
    }

    /* Too many open files */
    fs_close(real_fd);
    return -1;
}

/* Close file */
int sys_close(int fd) {
    if (fd == STDIN_FD || fd == STDOUT_FD || fd == STDERR_FD) {
        return 0;  /* Can't close standard streams */
    }

    if (fd >= FIRST_USER_FD && fd < (FIRST_USER_FD + KERNEL_MAX_FILES)) {
        int real_fd = g_kernel_fds[fd - FIRST_USER_FD];
        if (real_fd >= 0) {
            int result = fs_close(real_fd);
            g_kernel_fds[fd - FIRST_USER_FD] = -1;
            return result;
        }
    }

    return -1;
}

/* Get current process ID */
uint32_t sys_getpid(void) {
    process_t* proc = process_current();
    if (proc) {
        return proc->pid;
    }
    return 0;
}

/* Kill process */
int sys_kill(uint32_t pid, int signal) {
    (void)signal;  /* Signal number not fully used */
    return process_kill(pid);
}

/* Pipe (stub) */
int sys_pipe(int* fds) {
    (void)fds;
    return -1;  /* Not implemented yet */
}

/* Seek in file */
int sys_seek(int fd, uint32_t offset) {
    if (fd >= FIRST_USER_FD && fd < (FIRST_USER_FD + KERNEL_MAX_FILES)) {
        int real_fd = g_kernel_fds[fd - FIRST_USER_FD];
        if (real_fd >= 0) {
            return fs_seek(real_fd, offset);
        }
    }
    return -1;
}
