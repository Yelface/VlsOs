#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

/* System Call Interface (int 0x80) */

/* Syscall numbers */
#define SYS_EXIT        1
#define SYS_WRITE       4
#define SYS_READ        3
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_GETPID      20
#define SYS_KILL        37
#define SYS_PIPE        42
#define SYS_FORK        2
#define SYS_EXEC        11
#define SYS_WAIT        7
#define SYS_SEEK        19
#define SYS_STAT        18
#define SYS_MKDIR       39
#define SYS_RMDIR       40
#define SYS_UNLINK      10

/* Standard file descriptors */
#define STDIN           0
#define STDOUT          1
#define STDERR          2
#define FIRST_USER_FD   3

/* File open modes */
#define O_RDONLY        0x00
#define O_WRONLY        0x01
#define O_RDWR          0x02
#define O_CREAT         0x04
#define O_TRUNC         0x08
#define O_APPEND        0x10

/* CPU context for syscalls */
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
    uint32_t ds;
    uint32_t es;
} syscall_context_t;

/* Syscall handler */
void syscall_handler(syscall_context_t* ctx);

/* Wrapper functions for user programs */

/* Exit process with code */
void sys_exit(int code);

/* Write to file descriptor */
int sys_write(int fd, const void* buffer, uint32_t count);

/* Read from file descriptor */
int sys_read(int fd, void* buffer, uint32_t count);

/* Open file */
int sys_open(const char* filename, uint32_t flags);

/* Close file */
int sys_close(int fd);

/* Get current process ID */
uint32_t sys_getpid(void);

/* Kill process */
int sys_kill(uint32_t pid, int signal);

/* Fork current process */
int sys_fork(void);

/* Execute program */
int sys_exec(const char* filename, const char* argv[]);

/* Wait for child process */
int sys_wait(int* status);

/* Create pipe */
int sys_pipe(int* fds);

/* Seek in file */
int sys_seek(int fd, uint32_t offset);

#endif
