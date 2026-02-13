# VlsOs Kernel API Reference

## Core Functions

### Kernel Initialization

```c
void kmain(uint32_t magic, uint32_t addr);
```
- **Purpose**: Main kernel entry point called by bootloader
- **Parameters**:
  - `magic`: Multiboot magic number (0x2BADB002)
  - `addr`: Multiboot information structure address
- **Returns**: Never returns (kernel runs forever)
- **Called from**: Bootloader (multiboot.asm)

### Panic Handler

```c
void kernel_panic(const char* message);
```
- **Purpose**: Halt system with error message
- **Parameters**:
  - `message`: Panic message to display
- **Returns**: Never (halts CPU)
- **Usage**: Called on fatal errors
- **Example**:
  ```c
  kernel_panic("Out of memory!");
  ```

## VGA Display Driver

### Initialization

```c
void vga_init(void);
```
- Initializes VGA text mode driver (80x25)
- Call once at startup

### Display Output

```c
void vga_write_char(char c);
```
- Write single character to screen
- Handles newline, tab, backspace
- Auto-scrolls on overflow

```c
void vga_write_string(const char* str);
```
- Write null-terminated string
- Example: `vga_write_string("Hello World\n");`

```c
void vga_clear_screen(void);
```
- Clear entire screen and reset cursor

### Color Control

```c
void vga_set_color(uint8_t foreground, uint8_t background);
```
- Set text color for future output
- Parameters are color constants:
  ```c
  #define VGA_COLOR_BLACK         0
  #define VGA_COLOR_BLUE          1
  #define VGA_COLOR_GREEN         2
  #define VGA_COLOR_CYAN          3
  #define VGA_COLOR_RED           4
  #define VGA_COLOR_MAGENTA       5
  #define VGA_COLOR_BROWN         6
  #define VGA_COLOR_LIGHT_GREY    7
  #define VGA_COLOR_DARK_GREY     8
  #define VGA_COLOR_LIGHT_BLUE    9
  #define VGA_COLOR_LIGHT_GREEN   10
  #define VGA_COLOR_LIGHT_CYAN    11
  #define VGA_COLOR_LIGHT_RED     12
  #define VGA_COLOR_LIGHT_MAGENTA 13
  #define VGA_COLOR_YELLOW        14
  #define VGA_COLOR_WHITE         15
  ```
- Example:
  ```c
  vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
  vga_write_string("Success!\n");
  ```

### Cursor Management

```c
int vga_get_cursor_x(void);
int vga_get_cursor_y(void);
```
- Get current cursor position
- Returns: Column (0-79) and row (0-24)

## Keyboard Driver

### Initialization

```c
void keyboard_init(void);
```
- Initialize PS/2 keyboard driver

### Input

```c
char keyboard_read_char(void);
```
- Read character from keyboard (blocking)
- Converts PS/2 scan codes to ASCII
- Returns: ASCII character or special code
- Blocking: Waits for keyboard input
- Supports: US keyboard layout

## Timer (PIT) Driver

### Initialization

```c
void pit_init(uint32_t frequency);
```
- Initialize Programmable Interval Timer
- Parameters:
  - `frequency`: Interrupt frequency in Hz (typically 1000)

### Timing

```c
uint32_t pit_get_ticks(void);
```
- Get system ticks since init
- Returns: Tick count
- Updates every timer interrupt

```c
void pit_wait_ms(uint32_t ms);
```
- Wait specified milliseconds
- Parameters:
  - `ms`: Time to wait in milliseconds
- Blocking: Halts CPU until timeout

## Memory Management

### Allocation

```c
void* malloc(size_t size);
```
- Allocate memory from heap
- Parameters:
  - `size`: Number of bytes to allocate
- Returns: Pointer to allocated memory or NULL
- Note: Simple allocator - no free support

```c
void free(void* ptr);
```
- Free allocated memory (no-op in current implementation)
- For future compatibility

### Memory Operations

```c
void* memcpy(void* dest, const void* src, size_t n);
```
- Copy memory region
- Parameters:
  - `dest`: Destination address
  - `src`: Source address
  - `n`: Number of bytes to copy
- Returns: `dest`

```c
void* memset(void* s, int c, size_t n);
```
- Fill memory with value
- Parameters:
  - `s`: Memory address
  - `c`: Fill value (as unsigned char)
  - `n`: Number of bytes to fill
- Returns: `s`

```c
int memcmp(const void* s1, const void* s2, size_t n);
```
- Compare memory regions
- Parameters:
  - `s1`, `s2`: Memory addresses to compare
  - `n`: Number of bytes
- Returns: 0 if equal, <0 if s1<s2, >0 if s1>s2

## String Functions

### Length and Comparison

```c
size_t strlen(const char* str);
```
- Get string length (without null terminator)

```c
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
```
- Compare strings
- Returns: 0 if equal, <0 or >0 if different

### Manipulation

```c
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strchr(const char* s, int c);
```
- Copy, concatenate, find character in string

## Character Functions

```c
int isalpha(int c);      // Check if letter
int isdigit(int c);      // Check if digit
int isalnum(int c);      // Check if alphanumeric
int isspace(int c);      // Check if whitespace
int isupper(int c);      // Check if uppercase
int islower(int c);      // Check if lowercase
int toupper(int c);      // Convert to uppercase
int tolower(int c);      // Convert to lowercase
```

## Interrupt Handling

### IDT Setup

```c
void idt_init(void);
```
- Initialize Interrupt Descriptor Table
- Called early during kernel startup
- Sets up exception and IRQ handlers

### Exception Handlers

```c
void isr_handler(uint32_t isrnum);
```
- CPU exception handler
- Parameters:
  - `isrnum`: Exception/interrupt number

### IRQ Handlers

```c
void irq_handler(uint32_t irqnum);
```
- Hardware interrupt handler
- Parameters:
  - `irqnum`: IRQ number (0-15)

## Shell

### Main Loop

```c
void shell_main(void);
```
- Interactive shell command loop
- Displays prompt "vlsos> "
- Reads and executes commands
- Built-in commands:
  - `help` - Show available commands
  - `echo` - Echo arguments
  - `clear` - Clear screen
  - `uptime` - Show system ticks
  - `ls` - List files (simulation)
  - `exit` - Exit shell and halt

## Type Definitions

```c
#include "types.h"

typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
typedef unsigned char      bool;
#define true               1
#define false              0
#define NULL               ((void*)0)
```

## Common Patterns

### Initialize Kernel

```c
void kmain(uint32_t magic, uint32_t addr) {
    asm volatile("cli");           // Disable interrupts
    vga_init();
    vga_clear_screen();
    vga_write_string("VlsOs\n");
    
    memory_init();
    idt_init();
    pit_init(1000);
    keyboard_init();
    
    asm volatile("sti");           // Enable interrupts
    shell_main();
    kernel_panic("Shell exited!");
}
```

### Safe malloc with error checking

```c
void* buffer = malloc(1024);
if (!buffer) {
    kernel_panic("Memory allocation failed!");
}
// Use buffer...
```

### Colored output

```c
vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
vga_write_string("Error: ");
vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
vga_write_string("Something went wrong\n");
```

### Timed wait

```c
pit_init(1000);  // 1000 Hz timer
pit_wait_ms(500);  // Wait 500 milliseconds
```

## Compilation

Include necessary headers:
```c
#include "kernel.h"     // Core kernel functions
#include "drivers.h"    // Hardware drivers
#include "memory.h"     // Memory functions
#include "string.h"     // String functions
#include "types.h"      // Type definitions
```

Compile with:
```bash
i686-elf-gcc -ffreestanding -Wall -std=c99 -Iinclude -c yourfile.c
```

Link with:
```bash
i686-elf-ld -T linker.ld -o kernel.bin *.o
```
