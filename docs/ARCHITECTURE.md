# VlsOs Architecture Documentation

## System Architecture Overview

VlsOs is a 32-bit x86 protected mode operating system with the following components:

### Boot Process

1. **BIOS** - Firmware initializes and jumps to bootloader
2. **GRUB Bootloader** - Loads kernel at 0x100000, passes control to kernel entry
3. **Kernel Entry** - Assembly code in multiboot.asm sets up initial stack
4. **Kernel Main** - C function kmain() initializes hardware and starts shell

### Memory Layout (Physical)

```
0x00000000 - 0x000003FF : IVT (Interrupt Vector Table)
0x00000400 - 0x000004FF : BIOS Data Area
0x00000500 - 0x0009FBFF : Free / Kernel code (before 1MB)
0x0009FC00 - 0x0009FFFF : EBDA (Extended BIOS Data Area)
0x000F0000 - 0x000FFFFF : ROM/System BIOS
0x00100000 - 0x0019FFFF : Kernel binary space
0x00200000 - 0x002FFFFF : Heap (1 MB)
```

### Kernel Components

#### 1. Bootloader (src/boot/multiboot.asm)
- Multiboot header for bootloader compatibility
- Stack initialization
- Kernel entry point

#### 2. Core Kernel (src/kernel/main.c)
- Hardware initialization
- Driver initialization
- Shell startup
- Kernel panic handler

#### 3. Memory Management (src/kernel/memory.c)
- Simple heap allocator
- memcpy, memset, memcmp functions
- Virtual memory stubs

#### 4. Interrupt Handling (src/kernel/idt.c, interrupts.asm)
- Interrupt Descriptor Table (IDT) setup
- Exception handlers (divide by zero, page fault, etc.)
- IRQ handlers (timer, keyboard)

#### 5. VGA Driver (src/kernel/vga.c)
- Text mode (80x25) output
- Color support (16 colors)
- Cursor management
- Scroll on overflow

#### 6. Keyboard Driver (src/kernel/keyboard.c)
- PS/2 keyboard input
- Scancode to ASCII conversion
- US keyboard layout support

#### 7. Timer Driver (src/kernel/pit.c)
- Programmable Interval Timer (PIT)
- Interrupt-based timing
- Tick counter

#### 8. Standard Library (src/libc/string.c)
- String functions: strlen, strcpy, strcmp, etc.
- Character functions: isalpha, toupper, tolower, etc.
- Number conversion: atoi, itoa

#### 9. Shell (src/shell/shell.c)
- Interactive command interpreter
- Built-in commands: help, echo, clear, uptime, ls, exit
- Command parsing and execution
- Prompt and keyboard input handling

### Interrupt Handling

**Available Interrupts:**
- INT 0: Divide by zero
- INT 1: Debug exception
- INT 8: Double fault
- INT 14: Page fault
- INT 32 (IRQ 0): Timer (PIT)
- INT 33 (IRQ 1): Keyboard

### Protected Mode

VlsOs runs entirely in 32-bit protected mode:
- Global Descriptor Table (GDT) setup by GRUB
- Kernel code segment: 0x08
- Kernel data segment: 0x10
- Privilege level: Ring 0 (kernel mode)

### Thread and Process Model

Currently: Single-threaded, no multitasking
- Kernel runs in infinite loop handling interrupts
- Shell runs in main execution thread
- Timer and keyboard handled via interrupts

### Filesystem (Not Implemented)

- Planned: Simple FAT12 or VFS
- Current: Simulation in shell (ls command)

## Building

See README.md for build instructions.

## Future Enhancements

1. Paging and virtual memory
2. Process creation and scheduling
3. Filesystem (VFS + FAT12)
4. Device drivers (disk, network)
5. System calls interface
6. User mode execution
7. Multithreading
8. Memory protection
