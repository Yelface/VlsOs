#include "kernel.h"
#include "drivers.h"
#include "disk.h"
#include "memory.h"
#include "types.h"
#include "shell.h"

/* Forward declarations */
void idt_init(void);

/* Kernel entry point called from bootloader */
void kmain(uint32_t magic, uint32_t addr) {
	(void) addr;  /* Unused parameter */
	/* Disable interrupts during initialization */
	__asm__ volatile("cli");

	/* Initialize hardware drivers */
	vga_init();
	vga_clear_screen();
	vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);

	/* Print welcome message */
	vga_write_string("VlsOs Operating System v0.1.0\n");
	vga_write_string("================================\n\n");

	/* Verify multiboot magic */
	if (magic != 0x2BADB002) {
		kernel_panic("Invalid multiboot magic number!");
	}

	vga_write_string("Initializing kernel...\n");

	/* Initialize memory management */
	memory_init();
	vga_write_string("Memory management initialized\n");

	/* Initialize interrupt descriptor table */
	idt_init();
	vga_write_string("Interrupt handler initialized\n");

	/* Initialize timer */
	pit_init(1000);  /* 1000 Hz */
	vga_write_string("Timer initialized\n");

	/* Initialize keyboard */
	keyboard_init();
	vga_write_string("Keyboard initialized\n");

	/* Initialize disk subsystem */
	disk_init();
	vga_write_string("Disk subsystem initialized\n");

	/* Enable interrupts */
	__asm__ volatile("sti");

	vga_write_string("\nKernel initialization complete!\n");
	vga_write_string("Type 'help' for available commands.\n\n");
	vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	/* Start the shell */
	shell_main();

	/* If shell returns, kernel panic */
	kernel_panic("Shell exited unexpectedly!");
}

/* Kernel panic - halt execution */
void kernel_panic(const char* message) {
	vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
	vga_write_string("\nKERNEL PANIC: ");
	vga_write_string(message);
	vga_write_string("\n");

	/* Halt CPU */
	__asm__ volatile("cli; hlt");

	/* Should never reach here */
	while (1) {
		__asm__ volatile("hlt");
	}
}
