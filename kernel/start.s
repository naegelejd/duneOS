; vim:syntax=nasm
bits 32
align 4

; Definitions
KERNEL_CS equ 1 << 3
KERNEL_DS equ 2 << 3

STACK_SIZE          equ 0x1000 ; 4KB stack
THREAD_CONTEXT_SIZE equ 0x1000 ; 4KB just for thread struct

MBOOT_PAGE_ALIGN    equ 0x1
MBOOT_MEM_INFO      equ 0x2
MBOOT_USE_GFX       equ 0x4
MBOOT_HDR_MAGIC     equ 0x1BADB002
MBOOT_HDR_FLAGS     equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HDR_MAGIC + MBOOT_HDR_FLAGS)


; start of kernel image:
; Multiboot header
; note: you don't need Multiboot AOUT Kludge for an ELF kernel
multiboot:
    dd MBOOT_HDR_MAGIC
    dd MBOOT_HDR_FLAGS
    dd MBOOT_CHECKSUM
    ; Mem info (only valid if aout kludge flag set or ELF kernel)
    dd 0x00000000   ; header address
    dd 0x00000000   ; load address
    dd 0x00000000   ; load end address
    dd 0x00000000   ; bss end address
    dd 0x00000000   ; entry address
    ; Graphics requests (only valid if graphics flag set)
    ;dd 0x00000000   ; linear graphics
    ;dd 0            ; width
    ;dd 0            ; height
    ;dd 32           ; set to 32


[extern main]
[global g_start]
g_start:
    mov esp, kernel_stack_top ; set up stack pointer
    push eax    ; push header magic
    push ebx    ; push header pointer
    cli
    call main

    cli
.hang:
    hlt
    jmp .hang

; Load the 6-byte GDT pointer from the address passed as a parameter
; this will set up segment registers then far jump
[global gdt_flush]  ; make linkable
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]      ; load our GDT pointer from 'gdt.c'
    mov ax, 0x10    ; offset into GDT for kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush ; offset into GDT for kernel code segment (far jump)
.flush:
    ret

; Load the 6-byte IDT pointer from the address passed as parameter
[global idt_flush]
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret


%macro isr_no_err 1
[global isr%1]
isr%1:
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro isr_err_code 1
[global isr%1]
isr%1:
    cli
    nop
    nop
    push dword %1
    jmp isr_common_stub
%endmacro

; Interrupt Service Routines (ISRs)
isr_no_err 0    ; Divison by Zero Exception
isr_no_err 1    ; Debug Exception
isr_no_err 2    ; Non-Maskable Interrupt Exception
isr_no_err 3    ; Breakpoint Exception
isr_no_err 4    ; Into Detected Overflow Exception
isr_no_err 5    ; Out of Bounds Exception
isr_no_err 6    ; Invalid Opcode Exception
isr_no_err 7    ; No Coprocessor Exception
isr_err_code 8  ; Double Fault Exception
isr_no_err 9    ; Coprocessor Segment Overrun Exception
isr_err_code 10 ; Bad TSS Exception
isr_err_code 11 ; Segment Not Present Exception
isr_err_code 12 ; Stack Fault Exception
isr_err_code 13 ; General Fault Protection Exception
isr_err_code 14 ; Page Fault Exception
isr_no_err 15   ; Unknown Interrupt Exception
isr_no_err 16   ; Coprocessor Fault Exception
isr_no_err 17   ; Alignment Check Exception
isr_no_err 18   ; Machine Check Exception

; Interrupts 19-31 are Intel Reserved and have no error code
; Interrupts 32-255 have no error code
%assign intno 19
%rep (256 - 19)
isr_no_err intno
%assign intno intno + 1
%endrep


[extern default_int_handler]
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
    mov eax, default_int_handler
    call eax        ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8      ; Cleans up pushed error code and pushed ISR number
    sti             ; re-enable interrupts
    iret            ; Pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP


; Macro for all IRQ handling code
; Push ISR number, which is 32 + IDT number because
; we remap IRQ 0-15 to IDT 32-47
%macro irq_handle 1
[global irq%1]
irq%1:
    cli
    push byte 0
    push byte 32+%1
    jmp irq_common_stub
%endmacro

irq_handle 0    ; IRQ 0 (IDT 32)
irq_handle 1    ; IRQ 1 (IDT 33)
irq_handle 2    ; IRQ 2 (IDT 34)
irq_handle 3    ; IRQ 3 (IDT 35)
irq_handle 4    ; IRQ 4 (IDT 36)
irq_handle 5    ; IRQ 5 (IDT 37)
irq_handle 6    ; IRQ 6 (IDT 38)
irq_handle 7    ; IRQ 7 (IDT 39)
irq_handle 8    ; IRQ 8 (IDT 40)
irq_handle 9    ; IRQ 9 (IDT 41)
irq_handle 10    ; IRQ 10 (IDT 42)
irq_handle 11    ; IRQ 11 (IDT 43)
irq_handle 12    ; IRQ 12 (IDT 44)
irq_handle 13    ; IRQ 13 (IDT 45)
irq_handle 14    ; IRQ 14 (IDT 46)
irq_handle 15    ; IRQ 14 (IDT 47)


[extern default_irq_handler]
; calls default_irq_handler defined in 'irq.c'
irq_common_stub:
    pusha           ; push EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10    ; load kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, default_irq_handler
    call eax        ; preserves EIP ?
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8      ; clean up pushed error code and ISR number
    sti             ; re-enable interrupts
    iret            ; pop CS, EIP, EFLAGS, SS, and ESP


; return contents of EFLAGS register
[global get_eflags]
get_eflags:
    pushfd      ; push eflags
    pop eax     ; pop contents into eax
    ret


section .bss
[global kernel_stack_bottom]
[global kernel_stack_top]
[global main_thread_addr]
main_thread_addr:
    resb THREAD_CONTEXT_SIZE    ; reserve 4K for main kernel thread struct
kernel_stack_bottom:
    resb STACK_SIZE     ; reserve 4KB for kernel stack
kernel_stack_top:
