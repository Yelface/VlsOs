/* VlsOs syscall stubs for utilities */

#include "types.h"

/* Simple system calls (stubs for now) */

int syscall_write(int fd, const char* buffer, size_t count) {
	(void) fd;
	(void) buffer;
	(void) count;
	return -1;
}

int syscall_exit(int status) {
	(void) status;
	return -1;
}

int syscall_open(const char* filename, int flags) {
	(void) filename;
	(void) flags;
	return -1;
}

int syscall_close(int fd) {
	(void) fd;
	return -1;
}

int syscall_read(int fd, void* buffer, size_t count) {
	(void) fd;
	(void) buffer;
	(void) count;
	return -1;
}
