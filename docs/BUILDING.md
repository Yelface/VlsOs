# VlsOs Building and Installation Guide

## Prerequisites

### Required Tools

#### C Compiler (32-bit support)
Install GCC with 32-bit support for building i386 code:

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential
```

This includes:
- `gcc` - C compiler (with -m32 for 32-bit code)
- `ld` - Linker
- `make` - Build system

#### Assembler (NASM)
Install NASM for x86 assembly:

```bash
sudo apt-get install nasm
```

#### GRUB Bootloader Tools
```bash
sudo apt-get install grub-common xorriso
```

#### Optional: QEMU Emulator
For testing VlsOs without physical hardware:

```bash
sudo apt-get install qemu-system-x86
```

### Verify Installation

```bash
# Check compiler and build tools
gcc --version
ld --version
make --version

# Check assembler
nasm -version

# Check GRUB tools
grub-mkrescue --version

# Check QEMU (optional)
qemu-system-i386 --version
```

## Building VlsOs

### Quick Build

```bash
# Navigate to project directory
cd /path/to/VlsOs

# Build kernel
make

# Build ISO image
make iso
```

### Build Steps Explained

1. **Assemble Bootloader** - multiboot.asm → multiboot.o
   ```bash
   nasm -f elf32 src/boot/multiboot.asm -o build/multiboot.o
   ```

2. **Assemble Interrupt Handlers** - interrupts.asm → interrupts.o
   ```bash
   nasm -f elf32 src/kernel/interrupts.asm -o build/interrupts.o
   ```

3. **Compile Kernel Code** - *.c → *.o
   ```bash
   gcc -m32 -ffreestanding -fno-builtin -fno-stack-protector -Wall -std=c99 -Iinclude -nostartfiles -c src/kernel/main.c -o build/main.o
   (repeat for each .c file)
   ```

4. **Link Kernel** - *.o → vlsos.bin
   ```bash
   ld -m elf_i386 -T linker.ld -o build/vlsos.bin build/*.o
   ```

5. **Create Bootable ISO** - vlsos.bin → os.iso
   ```bash
   mkdir -p build/iso/boot/grub
   cp build/vlsos.bin build/iso/boot/
   # Create grub.cfg
   grub-mkrescue -o build/os.iso build/iso
   ```

### Build Artifacts

After building, you'll have:

```
build/
├── *.o              # Object files
├── vlsos.bin        # Kernel binary
├── os.iso           # Bootable ISO image
└── iso/             # ISO filesystem staging
    └── boot/
        ├── vlsos.bin
        └── grub/
            └── grub.cfg
```

## Running VlsOs

### In QEMU Emulator

```bash
# Build ISO first
make iso

# Run with default settings
make run

# Or manually:
qemu-system-i386 -cdrom build/os.iso
```

QEMU Options:
```bash
# With serial output
qemu-system-i386 -cdrom build/os.iso -serial mon:stdio

# With VNC display
qemu-system-i386 -cdrom build/os.iso -vnc :1

# With more RAM (default 128MB)
qemu-system-i386 -cdrom build/os.iso -m 256

# With curses display
qemu-system-i386 -cdrom build/os.iso -display curses
```

### On Physical Hardware

**Warning**: Running untested OS on real hardware can cause issues. Test in QEMU first.

1. Write ISO to USB:
   ```bash
   sudo dd if=build/os.iso of=/dev/sdX bs=4M status=progress
   ```
   (Replace sdX with your USB device)

2. Boot from USB (may require BIOS setup)

3. GRUB menu appears with "VlsOs" option

4. Select and boot

### Debugging with GDB

```bash
# Build with debug info
make DEBUG=1

# Start QEMU with GDB server
make debug

# Or manually:
qemu-system-i386 -cdrom build/os.iso -s -S &
gdb -ex "target remote localhost:1234" -ex "symbol-file build/vlsos.bin" build/vlsos.bin
```

GDB Commands:
```gdb
(gdb) break *0x100000          # Break at kernel start
(gdb) continue                  # Resume execution
(gdb) stepi                     # Step one instruction
(gdb) info registers            # Show CPU registers
(gdb) disassemble $eip          # Show assembly around EIP
(gdb) x/16x $esp                # Show stack contents
```

## Troubleshooting

### Build Errors

**Error: "i686-elf-gcc: command not found"**
- Install cross-compiler (see Prerequisites)
- Check PATH includes compiler bin directory

**Error: "linker.ld: No such file or directory"**
- Run make from project root directory
- Verify linker.ld exists in root

**Error: "grub-mkrescue: command not found"**
- Install GRUB tools: `sudo apt-get install grub-common xorriso`

### Runtime Errors

**QEMU won't boot**
- Check Multiboot header in multiboot.asm
- Verify kernel is properly linked
- Check GRUB configuration in Makefile

**Kernel panic on boot**
- Check memory addresses in linker.ld
- Verify interrupt handlers are registered
- Check hardware driver initialization

**Keyboard input not working**
- QEMU keyboard pass-through may be disabled
- Check keyboard driver initialization
- Try different QEMU display options

### Performance Issues

**Slow performance in QEMU**
- Allocate more CPU cores: `-smp 2`
- Allocate more RAM: `-m 256`
- Use host KVM acceleration: `-enable-kvm`

## Clean Build

To remove all build artifacts:

```bash
make clean
```

This removes the entire `build/` directory.

## Advanced Building

### Custom Compiler Flags

Edit Makefile CFLAGS variable:
```makefile
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -Wall -std=c99 -Iinclude -nostartfiles -O2
```

Key flags:
- `-m32`: Generate 32-bit x86 code
- `-ffreestanding`: Freestanding environment (no standard library)
- `-fno-builtin`: Don't use built-in functions
- `-fno-stack-protector`: Disable stack protection (not supported in freestanding)
- `-nostartfiles`: No startup files

### Custom Kernel Offset

Edit linker.ld:
```ld
. = 0x100000;  /* Change this */
```

### Custom Heap Size

Edit src/kernel/memory.c:
```c
#define HEAP_SIZE  0x200000  /* Change to 2 MB */
```

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make` or `make all` | Build kernel binary |
| `make iso` | Create bootable ISO image |
| `make run` | Run in QEMU |
| `make debug` | Debug in QEMU with GDB |
| `make clean` | Remove all build artifacts |
| `make help` | Show this help |

## Build Process Flow

```
multiboot.asm ─┐
interrupts.asm ├─→ Assembler ─┐
               │              ├─→ Linker → vlsos.bin
main.c ────────┤              │  (uses linker.ld)
vga.c ─────────├─→ Compiler ──┤
keyboard.c ────┤              │
pit.c ─────────┤              │
memory.c ──────┤              │
idt.c ─────────┤              │
string.c ──────┤              │
shell.c ───────┤              │
syscall.c ─────┘              │
                               ↓
                         Create ISO image
                              ↓
                            os.iso
                              ↓
                         Boot in QEMU
```

## Continuous Integration

To build automatically on save (requires `entr`):

```bash
find src/ include/ -name "*.c" -o -name "*.h" -o -name "*.asm" -o -name "*.ld" | \
  entr make clean && make iso && make run
```

## Next Steps

After successfully building and running VlsOs:
1. Read [ARCHITECTURE.md](ARCHITECTURE.md) for system overview
2. Check [MEMORY.md](MEMORY.md) for memory management details
3. Review [INTERRUPTS.md](INTERRUPTS.md) for interrupt handling
4. Look at kernel source code and add features
5. Develop new drivers and utilities
