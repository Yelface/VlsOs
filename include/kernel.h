#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

/* Kernel version and build info */
#define KERNEL_VERSION_MAJOR  0
#define KERNEL_VERSION_MINOR  1
#define KERNEL_VERSION_PATCH  0

/* Kernel API functions */
void kmain(uint32_t magic, uint32_t addr);
void kernel_panic(const char* message);

#endif
