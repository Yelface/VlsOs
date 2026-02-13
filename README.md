# VlsOs - A Real Operating System

VlsOs is a minimal but functional 32-bit x86 operating system written from scratch in assembly and C. It demonstrates core OS concepts including:

- **Bootloader**: GRUB Multiboot-compliant bootloader
- **Kernel**: Memory management, interrupt handling, process scheduling
- **Libc**: Basic C standard library for kernel and userspace
- **Shell**: Interactive command interpreter
- **Utilities**: Basic system tools (echo, help, ls simulation)

## Architecture

```
VlsOs (x86 32-bit)
├── Boot Stage (Multiboot)
├── Kernel Mode
│   ├── Memory Management (Paging & Virtual Memory)
│   ├── Interrupt Handling (IDT)
│   ├── Process Management
│   └── Drivers (VGA, PS/2, Timer)
└── User Mode
    ├── Shell
    └── Utilities
```

## Building

### Requirements
- `gcc` (with 32-bit support: `gcc -m32`)
- `nasm` (assembler)
- `ld` (linker)
- `make` (build system)
- `grub-mkrescue` (bootloader)
- `xorriso` (ISO creation)
- `qemu-system-i386` (optional, for testing)

### Build Instructions

```bash
# Build the kernel
make

# Create ISO image
make iso

# Run in QEMU (if installed)
make run

# Clean build artifacts
make clean
```

## File Structure

```
src/
  boot/          - Bootloader and kernel entry point
  kernel/        - Core kernel code
    - main.c     - Kernel entry point
    - memory.c   - Memory management
    - idt.c      - Interrupt descriptor table
    - pit.c      - Programmable interval timer
    - vga.c      - VGA text mode driver
    - keyboard.c - PS/2 keyboard driver
  libc/          - Standard C library
  shell/         - Command shell
  utils/         - System utilities
include/         - Header files
linker.ld        - Kernel linker script
Makefile         - Build configuration
```

## Running VlsOs

### With QEMU
```bash
qemu-system-i386 -cdrom build/os.iso
```

### With GRUB2 Bootloader
VlsOs uses GRUB2 as the bootloader via Multiboot specification, making it compatible with any Multiboot-compliant boot loader.

## Project Status

- [x] Bootloader (GRUB Multiboot)
- [x] Basic kernel structure
- [x] Memory management (paging)
- [x] Interrupt handling (IDT, ISR)
- [x] Timer driver (PIT)
- [x] VGA driver
- [x] Keyboard driver
- [x] Basic libc
- [x] Shell implementation
- [ ] Filesystem (planned)
- [ ] Processes and multitasking (basic)
- [ ] Networking (future)

## Documentation

See `docs/` directory for detailed documentation:
- `ARCHITECTURE.md` - System architecture
- `MEMORY.md` - Memory management details
- `INTERRUPTS.md` - Interrupt handling
- `KERNEL_API.md` - Kernel API reference
- `BUILDING.md` - Detailed build instructions

## License

This project is open source. Feel free to learn from, modify, and build upon it.

## References

- OSDev.org - https://wiki.osdev.org
- x86 Assembly - https://en.wikibooks.org/wiki/X86_Assembly
- Intel 80386 Reference Manual