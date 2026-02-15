; Interrupt handlers for VlsOs
; Exception handlers and IRQ handlers
; NASM syntax

BITS 32

section .text

; Port I/O helper functions
; void outb(uint16_t port, uint8_t value)
global outb
outb:
	mov dx, [esp + 4]     ; Get port from first argument
	mov al, [esp + 8]     ; Get value from second argument
	out dx, al
	ret

; uint8_t inb(uint16_t port)
global inb
inb:
	mov dx, [esp + 4]     ; Get port from first argument
	in al, dx             ; Read from port
	movzx eax, al         ; Zero-extend al to eax
	ret

; void outw(uint16_t port, uint16_t value)
global outw
outw:
	mov dx, [esp + 4]     ; Get port from first argument
	mov ax, [esp + 8]     ; Get value from second argument
	out dx, ax
	ret

; uint16_t inw(uint16_t port)
global inw
inw:
	mov dx, [esp + 4]     ; Get port from first argument
	in ax, dx             ; Read word from port
	movzx eax, ax         ; Zero-extend ax to eax
	ret

; Forward declarations
extern isr_handler
extern irq_handler

; Exception handlers
global isr0, isr1, isr8, isr14
global irq0, irq1

; Divide by zero exception
isr0:
	push dword 0
	push dword 0
	jmp isr_common_stub

; Debug exception
isr1:
	push dword 0
	push dword 1
	jmp isr_common_stub

; Double fault
isr8:
	push dword 8
	jmp isr_common_stub

; Page fault (error code pushed by CPU)
isr14:
	push dword 14
	jmp isr_common_stub

; IRQ 0 - Timer
irq0:
	push dword 0
	push dword 32
	jmp irq_common_stub

; IRQ 1 - Keyboard
irq1:
	push dword 0
	push dword 33
	jmp irq_common_stub

; Common exception handler
isr_common_stub:
	pusha
	mov ax, ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call isr_handler
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8
	iret

; Common IRQ handler
irq_common_stub:
	pusha
	mov ax, ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	call irq_handler
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8
	iret
