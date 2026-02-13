# VlsOs Interrupt Handling

## Overview

VlsOs implements interrupt and exception handling using the x86 Interrupt Descriptor Table (IDT). The system supports both CPU exceptions and hardware interrupts.

## Interrupt Types

### CPU Exceptions (INT 0-31)

Automatically triggered by the CPU for error conditions:

| INT | Name               | Description                        | Error Code |
|-----|--------------------|------------------------------------|------------|
| 0   | Divide Error       | Division by zero                   | No         |
| 1   | Debug Exception    | Breakpoint/single-step             | No         |
| 2   | NMI Interrupt      | Non-maskable interrupt             | No         |
| 3   | Breakpoint         | INT3 instruction                   | No         |
| 4   | Overflow           | INTO instruction overflow          | No         |
| 5   | BOUND Range        | BOUND instruction                  | No         |
| 6   | Invalid Opcode     | Undefined instruction              | No         |
| 7   | Device Not Found   | FPU not available                  | No         |
| 8   | Double Fault       | Multi-exception error              | Yes (0)    |
| 9   | Coprocessor Fault  | FPU protection error               | No         |
| 10  | Invalid TSS        | Invalid task switch segment        | Yes        |
| 11  | Segment Not Found  | Segment missing                    | Yes        |
| 12  | Stack Fault        | Stack segment problem              | Yes        |
| 13  | General Protection | Ring violation, memory errors      | Yes        |
| 14  | Page Fault         | Page not present / access error    | Yes        |
| 15  | Reserved           | (reserved by Intel)                | No         |
| 16  | FPU Error          | Floating-point error               | No         |
| 17  | Alignment Check    | Misaligned access                  | Yes        |
| 18  | Machine Check      | Internal machine error             | No         |
| 19  | SIMD Exception     | SSE/SSE2 error                     | No         |
| 20-31 | Reserved        | Reserved by Intel                  | -          |

Currently Implemented:
- INT 0: Divide by zero
- INT 1: Debug exception
- INT 8: Double fault
- INT 14: Page fault

### Hardware Interrupts (INT 32-47)

IRQs mapped through Programmable Interrupt Controller (PIC):

| IRQ | INT | Source           | Device    |
|-----|-----|------------------|-----------|
| 0   | 32  | Timer            | PIT       |
| 1   | 33  | Keyboard         | PS/2      |
| 2   | 34  | Cascade          | (PIC)     |
| 3   | 35  | COM2/COM4        | (planned) |
| 4   | 36  | COM1/COM3        | (planned) |
| 5   | 37  | LPT2             | (planned) |
| 6   | 38  | Floppy Disk      | (planned) |
| 7   | 39  | LPT1             | (planned) |
| 8   | 40  | CMOS Clock       | (planned) |
| 9   | 41  | Network          | (planned) |
| 10  | 42  | SCSI / Sound     | (planned) |
| 11  | 43  | SCSI             | (planned) |
| 12  | 44  | PS/2 Mouse       | (planned) |
| 13  | 45  | Coprocessor      | (planned) |
| 14  | 46  | IDE Primary      | (planned) |
| 15  | 47  | IDE Secondary    | (planned) |

Currently Implemented:
- IRQ 0 (INT 32): Timer (PIT)
- IRQ 1 (INT 33): Keyboard

## IDT Structure

### Interrupt Descriptor Table Entry (8 bytes)

```c
struct idt_entry {
    uint16_t base_lo;      // Low 16 bits of handler address
    uint16_t selector;     // Code segment selector (0x08 = kernel)
    uint8_t  zero;         // Always 0
    uint8_t  flags;        // Present, privilege level, type
    uint16_t base_hi;      // High 16 bits of handler address
} __attribute__((packed));
```

### Flags Byte

```
Bit: 7  6  5  4  3  2  1  0
    [P][DPL  ][0][GateType]

P (bit 7):    Present flag (1 = valid)
DPL (bits 6-5): Privilege level (0=kernel, 3=user)
Reserved (bit 4): Always 0
Gate Type (bits 3-0):
    0101 = Task gate
    0110 = Interrupt gate (disables further interrupts)
    0111 = Trap gate (allows nested interrupts)
```

For VlsOs kernel: flags = 0x8E (10001110)
- P = 1 (present)
- DPL = 0 (kernel only)
- GateType = 6 (interrupt gate)

## Interrupt Handling Flow

### Exception/Interrupt occurs
```
1. CPU pushes error code (if applicable)
2. CPU pushes return address (EIP)
3. CPU pushes code segment (CS)
4. CPU pushes flags (EFLAGS)
5. CPU loads handler address from IDT
6. CPU disables interrupts (for interrupt gates)
7. Handler code executes
8. Handler executes IRET
9. CPU pops EFLAGS, CS, EIP
10. CPU pops error code (if present)
11. Execution resumes
```

### Handler Execution (Assembly in interrupts.asm)

```asm
Handler:
    PUSH error_code/0      # Push error code or 0
    PUSH exception_number   # Push exception/interrupt number
    
    ; Save CPU state
    PUSHA                   # Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    MOV EAX, DS
    PUSH EAX                # Save data segment selector
    
    ; Load kernel data segment
    MOV AX, 0x10
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    
    ; Call C handler function
    CALL isr_handler / irq_handler
    
    ; Restore CPU state
    POP EAX
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    POPA
    
    ; Return
    ADD ESP, 8              # Skip error code and interrupt number
    IRET
```

## Programmed Interrupt Controller (PIC)

The 8259A PIC maps 16 hardware interrupts:

```
Master PIC (INT 32-39 = IRQ 0-7)
    Interrupt Base Address: 0x20
    Interrupt Vector:       0x21

Slave PIC (INT 40-47 = IRQ 8-15)
    Interrupt Base Address: 0xA0
    Interrupt Vector:       0xA1
```

### End of Interrupt (EOI)

Required after handling hardware interrupts:

```asm
MOV AL, 0x20    ; EOI command
OUT 0x20, AL    ; Send to Master PIC port
```

## Interrupt Nesting

### Disabled (Interrupt Gates)
- Interrupts automatically disabled during handler
- Prevents interrupt recursion
- Simpler but slower

### Enabled (Trap Gates)
- Interrupts allowed during handler
- Must be careful with stack
- Currently not used in VlsOs

## Performance Considerations

### Latency
- Exception: ~40-100 CPU cycles
- Interrupt: ~200-300 CPU cycles (due to PIC)

### Frequency
- Timer: 1000 Hz (1 interrupt per ms)
- Keyboard: Variable (user input)
- Total: Low interrupt load

## Common Issues

### Triple Fault
Caused by:
- Invalid IDT entry
- Handler crashes
- Stack corruption

Recovery: System reset required

### Interrupt Storm
Caused by:
- Handler doesn't acknowledge interrupt
- Interrupt line stuck low
- Infinite interrupt recursion

Recovery: Disable interrupt source

### Stack Overflow
Caused by:
- Very deep handler nesting
- Infinite recursion in handler

Recovery: Increase stack size, check code

## Future Enhancements

1. **Trap gates** - Allow interrupt nesting
2. **Task gates** - Hardware task switching
3. **Page fault handler** - Virtual memory support
4. **General protection fault** - Memory protection
5. **More IRQ handlers** - Disk, network, etc.
6. **APIC support** - Multi-processor IRQs
