; vim:syntax=nasm
[bits 32]

[extern main]
global start
start:
    call main
    jmp $

; extern void gdt_flush() in C
; this will set up segment registers then far jump 
global gdt_flush    ; make linkable
extern gp           ; defined in 'gdt.c'
gdt_flush:
    lgdt [gp]       ; load our GDT pointer from 'gdt.c'
    mov ax, 0x10    ; 0x10 is offset into GDT for data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2 ; 0x08 is offset to code segment (far jump)
flush2:
    ret

; extern void idt_load() in C
; this will load the IDT defined in 'idt.c'
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; Interrupt Service Routines (ISRs)
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

isr0:       ; Division by Zero Exception
    cli
    push byte 0 ; dummy error code
    push byte 0
    jmp isr_common_stub

isr1:       ; Debug Exception
    cli
    push byte 0 ; dummy error code
    push byte 1
    jmp isr_common_stub

isr2:       ; Non Maskable Interrupt Exception
    cli
    push byte 0 ; dummy error code
    push byte 2
    jmp isr_common_stub

isr3:       ; Breakpoint Exception
    cli
    push byte 0 ; dummy error code
    push byte 3
    jmp isr_common_stub

isr4:       ; Into Detected Overflow Exception
    cli
    push byte 0 ; dummy error code
    push byte 4
    jmp isr_common_stub

isr5:       ; Out of Bounds Exception
    cli
    push byte 0 ; dummy error code
    push byte 5
    jmp isr_common_stub

isr6:       ; Invalid Opcode Exception
    cli
    push byte 0 ; dummy error code
    push byte 6
    jmp isr_common_stub

isr7:       ; No Coprocessor Exception
    cli
    push byte 0 ; dummy error code
    push byte 7
    jmp isr_common_stub

isr8:       ; Double Fault Exception
    cli
    ; pushes it's own error code
    push byte 8
    jmp isr_common_stub

isr9:       ; Coprocessor Segment Overrun Exception
    cli
    push byte 0 ; dummy error code
    push byte 9
    jmp isr_common_stub

isr10:      ; Bad TSS Exception
    cli
    ; pushes it's own error code
    push byte 10
    jmp isr_common_stub

isr11:      ; Segment Not Present Exception
    cli
    ; pushes it's own error code
    push byte 11
    jmp isr_common_stub

isr12:      ; Stack Fault Exception
    cli
    ; pushes it's own error code
    push byte 12
    jmp isr_common_stub

isr13:      ; General Fault Protection Exception
    cli
    ; pushes it's own error code
    push byte 13
    jmp isr_common_stub

isr14:      ; Page Fault Exception
    cli
    ; pushes it's own error code
    push byte 14
    jmp isr_common_stub

isr15:      ; Unknown Interrupt Exception
    cli
    push byte 0 ; dummy error code
    push byte 15
    jmp isr_common_stub

isr16:      ; Coprocessor Fault Exception
    cli
    push byte 0 ; dummy error code
    push byte 16
    jmp isr_common_stub

isr17:      ; Alignment Check Exception
    cli
    push byte 0 ; dummy error code
    push byte 17
    jmp isr_common_stub

isr18:      ; Machine Check Exception
    cli
    push byte 0 ; dummy error code
    push byte 18
    jmp isr_common_stub

isr19:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 19
    jmp isr_common_stub

isr20:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 20
    jmp isr_common_stub

isr21:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 21
    jmp isr_common_stub

isr22:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 22
    jmp isr_common_stub

isr23:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 23
    jmp isr_common_stub

isr24:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 24
    jmp isr_common_stub

isr25:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 25
    jmp isr_common_stub

isr26:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 26
    jmp isr_common_stub

isr27:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 27
    jmp isr_common_stub

isr28:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 28
    jmp isr_common_stub

isr29:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 29
    jmp isr_common_stub

isr30:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 30
    jmp isr_common_stub

isr31:      ; Reserved
    cli
    push byte 0 ; dummy error code
    push byte 31
    jmp isr_common_stub

extern fault_handler
; Common ISR stub
; Save processor state, set up for kernel mode segments,
; call C-level fault handler, restore processor state
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10    ; Load the Kernel Data Segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp    ; Push the stack
    push eax
    mov eax, fault_handler
    call eax        ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8      ; Cleans up pushed error code and pushed ISR number
    iret            ; Pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP


global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; IRQ 0 (IDT 32)
irq0:
    cli
    push byte 0     ; push dummy error code
    push byte 32
    jmp irq_common_stub

; IRQ 1 (IDT 33)
irq1:
    cli
    push byte 0     ; push dummy error code
    push byte 33
    jmp irq_common_stub

; IRQ 2 (IDT 34)
irq2:
    cli
    push byte 0     ; push dummy error code
    push byte 34
    jmp irq_common_stub

; IRQ 3 (IDT 35)
irq3:
    cli
    push byte 0     ; push dummy error code
    push byte 35
    jmp irq_common_stub

; IRQ 4 (IDT 36)
irq4:
    cli
    push byte 0     ; push dummy error code
    push byte 36
    jmp irq_common_stub

; IRQ 5 (IDT 37)
irq5:
    cli
    push byte 0     ; push dummy error code
    push byte 37
    jmp irq_common_stub

; IRQ 6 (IDT 38)
irq6:
    cli
    push byte 0     ; push dummy error code
    push byte 38
    jmp irq_common_stub

; IRQ 7 (IDT 39)
irq7:
    cli
    push byte 0     ; push dummy error code
    push byte 39
    jmp irq_common_stub

; IRQ 8 (IDT 40)
irq8:
    cli
    push byte 0     ; push dummy error code
    push byte 40
    jmp irq_common_stub

; IRQ 9 (IDT 41)
irq9:
    cli
    push byte 0     ; push dummy error code
    push byte 41
    jmp irq_common_stub

; IRQ 10 (IDT 42)
irq10:
    cli
    push byte 0     ; push dummy error code
    push byte 42
    jmp irq_common_stub

; IRQ 11 (IDT 43)
irq11:
    cli
    push byte 0     ; push dummy error code
    push byte 43
    jmp irq_common_stub

; IRQ 12 (IDT 44)
irq12:
    cli
    push byte 0     ; push dummy error code
    push byte 44
    jmp irq_common_stub

; IRQ 13 (IDT 45)
irq13:
    cli
    push byte 0     ; push dummy error code
    push byte 45
    jmp irq_common_stub

; IRQ 14 (IDT 46)
irq14:
    cli
    push byte 0     ; push dummy error code
    push byte 46
    jmp irq_common_stub

; IRQ 15 (IDT 47)
irq15:
    cli
    push byte 0     ; push dummy error code
    push byte 47
    jmp irq_common_stub

extern default_irq_handler
; calls default_irq_handler defined in 'irq.c'
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, default_irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

