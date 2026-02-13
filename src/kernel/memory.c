#include "memory.h"
#include "types.h"

/* Simple memory allocator - very basic for now */

#define HEAP_START 0x200000
#define HEAP_SIZE  0x100000  /* 1 MB heap */
#define HEAP_END   (HEAP_START + HEAP_SIZE)

static uint8_t* heap = (uint8_t*) HEAP_START;
static uint8_t* heap_ptr = (uint8_t*) HEAP_START;

/* Initialize memory management */
void memory_init(void) {
	heap_ptr = heap;
}

/* Allocate memory from heap */
void* malloc(size_t size) {
	if (heap_ptr + size >= (uint8_t*) HEAP_END) {
		return NULL;  /* Out of memory */
	}

	void* ptr = heap_ptr;
	heap_ptr += size;
	return ptr;
}

/* Free memory (no-op for now) */
void free(void* ptr) {
	(void) ptr;  /* Not implemented - no memory reclamation yet */
}

/* Copy memory */
void* memcpy(void* dest, const void* src, size_t n) {
	uint8_t* d = (uint8_t*) dest;
	const uint8_t* s = (const uint8_t*) src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	}

	return dest;
}

/* Compare memory */
int memcmp(const void* s1, const void* s2, size_t n) {
	const uint8_t* a = (const uint8_t*) s1;
	const uint8_t* b = (const uint8_t*) s2;

	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			return a[i] - b[i];
		}
	}

	return 0;
}

/* Set memory */
void* memset(void* s, int c, size_t n) {
	uint8_t* p = (uint8_t*) s;

	for (size_t i = 0; i < n; i++) {
		p[i] = (uint8_t) c;
	}

	return s;
}

/* Paging stub - not fully implemented */
void paging_init(void) {
	/* TODO: Implement paging */
}

void paging_enable(void) {
	/* TODO: Enable paging in CR0 */
}

uint32_t virt_to_phys(uint32_t virt_addr) {
	/* Identity mapping for now */
	return virt_addr;
}

uint32_t phys_to_virt(uint32_t phys_addr) {
	/* Identity mapping for now */
	return phys_addr;
}

/* Virtual memory allocation (same as malloc for now) */
void* vmalloc(size_t size) {
	return malloc(size);
}

void vfree(void* ptr) {
	free(ptr);
}
