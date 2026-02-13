; Multiboot bootloader header and kernel entry point
; VlsOs Bootloader - NASM syntax

BITS 32

; Multiboot header definition
section .multiboot
align 4
	dd 0x1BADB002              ; Magic number
	dd 0x00000003              ; Flags
	dd -(0x1BADB002 + 0x00000003)  ; Checksum

; Temporary stack in boot section
section .bss
align 16
stack_bottom:
	resb 16384  ; 16 KB stack
stack_top:

section .text
extern kmain

global start
start:
	; Set up the stack
	mov esp, stack_top

	; Push multiboot info (arguments to kmain)
	push ebx            ; multiboot info structure
	push eax            ; multiboot magic number

	; Call kernel main function
	call kmain

	; Hang if kernel returns
	cli

.halt:
	hlt
	jmp .halt
