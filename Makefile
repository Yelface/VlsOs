# VlsOs Makefile
# Compiler and tools configuration

CC = gcc
AS = nasm
LD = ld
AR = ar

# Compiler flags for 32-bit x86
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -std=c99 -Iinclude -nostartfiles
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

# Directories
SRC_DIR = src
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub

# Output files
KERNEL = $(BUILD_DIR)/vlsos.bin
ISO = $(BUILD_DIR)/os.iso

# Source files
KERNEL_SOURCES = \
	src/boot/multiboot.asm \
	src/kernel/interrupts.asm \
	src/kernel/main.c \
	src/kernel/vga.c \
	src/kernel/keyboard.c \
	src/kernel/pit.c \
	src/kernel/memory.c \
	src/kernel/idt.c \
	src/kernel/disk.c \
	src/kernel/process.c \
	src/libc/string.c \
	src/shell/shell.c \
	src/utils/syscall.c \
	src/net/net.c \
	src/net/arp.c \
	src/net/ip.c \
	src/net/icmp.c \
	src/net/udp.c \
	src/net/tcp.c \
	src/net/netdrv.c \
	src/net/socket.c \
	src/net/netcmd.c \
	src/net/http.c \
	src/net/dns.c \
	src/net/dhcp.c \
	src/net/http_client.c \
	src/ui/ui.c

KERNEL_OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(KERNEL_SOURCES:.c=.o)))
KERNEL_OBJS := $(KERNEL_OBJS:.asm=.o)

# Default target
all: $(KERNEL)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile C sources
$(BUILD_DIR)/%.o: src/*/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble assembly sources
$(BUILD_DIR)/%.o: src/*/%.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Special case for multiboot.asm in src/boot
$(BUILD_DIR)/multiboot.o: src/boot/multiboot.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Special case for interrupts.asm in src/kernel
$(BUILD_DIR)/interrupts.o: src/kernel/interrupts.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Build kernel
$(KERNEL): $(BUILD_DIR)/multiboot.o $(BUILD_DIR)/interrupts.o \
           $(BUILD_DIR)/main.o $(BUILD_DIR)/vga.o $(BUILD_DIR)/keyboard.o \
           $(BUILD_DIR)/pit.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/idt.o \
           $(BUILD_DIR)/disk.o $(BUILD_DIR)/process.o $(BUILD_DIR)/string.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/syscall.o \
           $(BUILD_DIR)/net.o $(BUILD_DIR)/arp.o $(BUILD_DIR)/ip.o \
           $(BUILD_DIR)/icmp.o $(BUILD_DIR)/udp.o $(BUILD_DIR)/tcp.o \
$(BUILD_DIR)/netdrv.o $(BUILD_DIR)/socket.o $(BUILD_DIR)/netcmd.o $(BUILD_DIR)/http.o $(BUILD_DIR)/dns.o $(BUILD_DIR)/dhcp.o $(BUILD_DIR)/http_client.o $(BUILD_DIR)/ui.o
	$(LD) $(LDFLAGS) -o $@ $^

# Build ISO image (VirtualBox-compatible with BIOS boot)
iso: $(KERNEL)
	mkdir -p $(GRUB_DIR)
	cp $(KERNEL) $(BOOT_DIR)/vlsos.bin
	echo 'set default=0' > $(GRUB_DIR)/grub.cfg
	echo 'set timeout=5' >> $(GRUB_DIR)/grub.cfg
	echo '' >> $(GRUB_DIR)/grub.cfg
	echo 'menuentry "VlsOs" {' >> $(GRUB_DIR)/grub.cfg
	echo '  multiboot /boot/vlsos.bin' >> $(GRUB_DIR)/grub.cfg
	echo '  boot' >> $(GRUB_DIR)/grub.cfg
	echo '}' >> $(GRUB_DIR)/grub.cfg
	grub-mkrescue --compress=xz -o $(ISO) $(ISO_DIR) 2>&1 || { \
		echo "ERROR: Failed to create ISO with grub-mkrescue"; \
		exit 1; \
	}
	@if [ -f $(ISO) ]; then \
		echo "✓ ISO image created: $(ISO)"; \
		ls -lh $(ISO); \
	fi

# Run in QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO) -monitor stdio

# Run with GDB debugging
debug: iso
	qemu-system-i386 -cdrom $(ISO) -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL)" $(KERNEL)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Display help
help:
	@echo "VlsOs Makefile targets:"
	@echo "  make           - Build the kernel"
	@echo "  make iso       - Create bootable ISO image"
	@echo "  make run       - Run in QEMU"
	@echo "  make debug     - Debug with GDB"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make help      - Display this help message"

.PHONY: all iso run debug clean help
