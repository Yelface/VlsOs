#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory management */
void memory_init(void);
void* malloc(size_t size);
void  free(void* ptr);
void* memcpy(void* dest, const void* src, size_t n);
int   memcmp(const void* s1, const void* s2, size_t n);
void* memset(void* s, int c, size_t n);

/* Paging structures and functions */
void paging_init(void);
void paging_enable(void);
uint32_t virt_to_phys(uint32_t virt_addr);
uint32_t phys_to_virt(uint32_t phys_addr);

/* Virtual memory management */
void* vmalloc(size_t size);
void  vfree(void* ptr);

#endif
