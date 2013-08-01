; A simple boot sector (no GRUB)

[org 0x7C00]

KERNEL_OFFSET equ 0x1000    ; memory offset to which the kernel is loaded

mov [BOOT_DRIVE], dl        ; store boot drive # for later

; Set the stack location
mov bp, 0x9000
mov sp, bp

;push cs
;pop ds  ; set data segment to code segment

mov si, MSG_REAL_MODE
call print_string   ; Announce that we're in 16-bit real mode
call load_kernel    ; Load kernel
call switch_to_pm   ; Announce 32-bit protected mode. This doesn't return
jmp $

[bits 16]

load_kernel:
    ; set up parameters for the disk_load routine
    ; load first XX sectors (after boot sector) from boot disk
    ; to ES:BX (0x0000:0x1000)
    xor ax, ax
    mov es, ax              ; ES = 0
    mov bx, KERNEL_OFFSET   ; BX = 0x1000
    mov dh, 32
    mov dl, [BOOT_DRIVE]
    call disk_load

    mov si, MSG_LOAD_KERNEL
    call print_string

    ret


[bits 32]

; we arrive here after switching to and initializing protected mode
BEGIN_PM:
    mov esi, MSG_PROT_MODE
    call print_string_pm    ; call 32-bit print routine
    call KERNEL_OFFSET      ; Jump to address of loaded kernel code
    jmp $           ; jump to the current address (infinite loop)


BOOT_DRIVE:
    db 0
MSG_REAL_MODE:
    db 'Started in 16-bit Real Mode...', 0
MSG_PROT_MODE:
    db 'Successfully landed in 32-bit Protected Mode', 0
MSG_LOAD_KERNEL:
    db 'Kernel loaded into memory.', 0


; print a string who's address is in SI
print_string:
    pusha
    mov ah, 0xE ; int 10/ah = 0eh -> scrolling teletype BIOS routine

.repeat:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .repeat

.done:
    popa
    ret


; load DH sectors to ES:BX from drive DL
disk_load:
    ; store DH (# of sectors to read)
    ; and DL (boot drive #)
    push dx

    mov ah, 0       ; BIOS reset floppy disk function
    int 0x13        ; BIOS interrupt
    jc reset_error  ; Jump if error (carry flag set)

    mov ah, 0x02    ; BIOS read sector function
    mov al, dh      ; Read DH sectors
    mov ch, 0       ; Select cylinder 0
    mov cl, 0x02    ; Start reading from 2nd sector (after boot sector)
    mov dh, 0       ; Select head 0
    mov dl, 0       ; Select drive 0 (floppy)

    int 0x13        ; BIOS interrupt
    jc disk_error   ; Jump if error (carry flag set)

    pop dx          ; Restore DX from stack
    cmp dh, al      ; if AL (sectors read) != DH (sectors expected)
    jne  disk_error
    ret

reset_error:
    mov si, RESET_ERROR_MSG
    call print_string
    jmp $

disk_error:
    mov si, DISK_ERROR_MSG
    call print_string
    jmp $

RESET_ERROR_MSG:
    db 'Disk reset error!', 0
DISK_ERROR_MSG:
    db 'Disk read error!', 0


; GDT
gdt_start:

; mandatory null descriptor
gdt_null:
    dd 0x0  ; 'dd' defines a double word (4 bytes)
    dd 0x0

; code segment descriptor
gdt_code:
    ; base = 0x0, limit = 0xffff
    ; 1st flags: (present)1 (privilege)00 (descriptor type)1 -> 1001b
    ; type flags: (code)1 (conforming)0 (readable)1 (accessed)0 -> 1010b
    ; 2nd flags: (granularity)1 (32-bit default)1 (64-bit seg)0 (AVL)0 -> 1100b
    dw 0xffff   ; Limit (bits 0-15)
    dw 0x0      ; Base (bits 0-15)
    db 0x0      ; Base (bits 16-23)
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0      ; Base (bits 24-31)

; the data segment descriptor
gdt_data:
    ; Same as code segment except for the type flags
    ; type flags: (code)0 (expand down)0 (writable)1 (accessed)0 -> 0010b
    dw 0xffff   ; Limit (bits 0-15)
    dw 0x0      ; Base (bits 0-15)
    db 0x0      ; Base (bits 16-23)
    db 10010010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0      ; Base (bits 24-31)

; Put a label at the end of the GDT so the assembler can calculate
; the size of the GDT for the GDT descriptor table (below)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size always 1 less than true size
    dd gdt_start                ; Start address of our GDT

; Define constants for the GDT segment descriptor offsets, which the
; segment registers must contain when in protected mode.
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


; Switch to protected mode
[bits 16]
switch_to_pm:

    ; Disable interrupts until the protected mode
    ; interrupt vector is installed
    cli

    ; Load global descriptor table, which defines the protected mode segments
    lgdt [gdt_descriptor]

    ; set first bit of CRO to switch to protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Make a far jump (to new segment) to the 32-bit code
    ; This forces the CPU to flush its cache of pre-fetched and real-mode
    ; decoded instructions, which can cause problems
    jmp CODE_SEG:init_pm

; Initialize registers and the stack once in PM
[bits 32]
init_pm:

    ; Now in PM, our old segments are meaningless
    ; so we point our segment registers to the
    ; data selector defined in the GDT
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Update the stack position so it's at the top of the free space
    mov ebp, 0x90000
    mov esp, ebp

    ; Call some well-known label
    call BEGIN_PM


VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; Prints a null-terminated string pointed to by ESI
[bits 32]
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY   ; Set EDX to start of video mem

repeat:
    mov al, [esi]       ; Store the char at ESI in AL
    mov ah, WHITE_ON_BLACK  ; Store attributes in AH

    cmp al, 0       ; if nul terminator is found, jump
    je done

    mov [edx], ax   ; Store char and attributes at current char cell
    add esi, 1      ; Move to next char in string
    add edx, 2      ; Move to next char cell in video mem

    jmp repeat

done:
    popa
    ret


; Padding and magic BIOS number
times 510 - ($ - $$) db 0   ; Pad the boot sector out to 510 bytes
dw 0xAA55               ; Last two bytes form the magic number,
                        ; so BIOS knows we are a boot sector

; vim: set syntax=nasm:
