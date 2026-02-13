# VlsOs Memory Management

## Overview

VlsOs implements a simple memory management system suitable for a minimalist kernel.

## Physical Memory Layout

The x86 memory addressing with VlsOs follows this layout:

```
Region                  Address Range         Size      Usage
================================================================================
Real Mode IVT          0x00000000-0x003FF     1 KB     Interrupt vectors (BIOS)
BIOS Data Area         0x00000400-0x004FF     256 B    System data (BIOS)
Free Memory            0x00000500-0x09FBFF    ~638 KB  Conventional memory
EBDA                   0x0009FC00-0x0009FFFF  16 KB    Extended BIOS data
ROM                    0x000F0000-0x000FFFFF  64 KB    System ROM/BIOS
High Memory            0x00100000+             > 1 MB   Extended memory

Kernel+Kernel Code     0x00100000-0x0019FFFF  1 MB     Kernel binary and stack
Heap                   0x00200000-0x002FFFFF  1 MB     Dynamic allocation
```

## Kernel Memory Regions

### Code Section (.text)
- **Address**: 0x100000 (1 MB)
- **Contains**: Executable kernel code (assembly + C compiled)
- **Permissions**: Read/Execute

### Read-Only Data (.rodata)
- **Address**: Follows code section
- **Contains**: String literals, constants
- **Permissions**: Read

### Initialized Data (.data)
- **Address**: Follows .rodata section
- **Contains**: Static variables with initial values
- **Permissions**: Read/Write

### Uninitialized Data (.bss)
- **Address**: Follows .data section
- **Contains**: Static variables (zero-initialized)
- **Permissions**: Read/Write

### Stack
- **Address**: 0x10000 (grows downward in boot)
- **Size**: 16 KB (declared in multiboot.asm)
- **Used by**: Multiboot handler and boot code

### Heap
- **Address**: 0x200000
- **Size**: 1 MB
- **Used by**: malloc/free allocations

## Memory Management Algorithms

### Heap Allocator

Current implementation uses a **simple linear allocator**:

```c
void* malloc(size_t size) {
    if (heap_ptr + size >= HEAP_END) {
        return NULL;  // Out of memory
    }
    void* ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}
```

**Characteristics:**
- Very fast allocation (O(1))
- No fragmentation during allocation
- No free (memory never reclaimed during runtime)
- Suitable for static kernel data structures

**Limitations:**
- Cannot free allocated memory
- Will eventually run out of heap space
- Not suitable for long-running applications

### Future: Advanced Allocators

Planned improvements:
1. **Buddy Allocator** - Fast allocation/deallocation with moderate fragmentation
2. **Slab Allocator** - Efficient for fixed-size objects
3. **Virtual Memory** - Paging to provide more apparent memory

## Standard C Memory Functions

### Provided Functions

```c
void* malloc(size_t size)       - Allocate size bytes
void  free(void* ptr)           - Free allocated memory (no-op)
void* memcpy(void*, const void*, size_t) - Copy memory
int   memcmp(const void*, const void*, size_t) - Compare memory
void* memset(void*, int, size_t) - Fill memory with value
```

## Paging (Future)

VlsOs has stubs for paging but doesn't currently use it:

```c
void paging_init(void)          - Initialize page tables
void paging_enable(void)        - Enable paging in CR0
uint32_t virt_to_phys(uint32_t) - Virtual to physical address
uint32_t phys_to_virt(uint32_t) - Physical to virtual address
```

When enabled, paging will provide:
- Virtual address space
- Memory protection between processes
- Demand paging
- Virtual memory > physical memory

## Debugging Memory Issues

### Memory Leaks
Currently not possible to detect due to simple allocator.

### Out of Memory
Results in malloc() returning NULL.

### Memory Corruption
No protection - undetected until crash.

## Configuration

Edit the following defines in `src/kernel/memory.c`:

```c
#define HEAP_START 0x200000  /* Starting address */
#define HEAP_SIZE  0x100000  /* Size in bytes (1 MB) */
#define HEAP_END   (HEAP_START + HEAP_SIZE)
```

## Memory Usage Statistics

Typical kernel footprint:
- Kernel binary: ~50-100 KB
- Static data: ~5-10 KB
- Stack: 16 KB
- Remaining heap: ~890 KB available

## Best Practices

1. **Minimize Dynamic Allocation** - Use static arrays where possible
2. **Avoid Memory Leaks** - Only allocate what's necessary
3. **Check Allocation Success** - Always test malloc() return value
4. **Use memset() Carefully** - Ensure correct size parameter
