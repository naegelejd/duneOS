; vim:syntax=nasm

section .text
align 4

global inportb
inportb:
    mov     edx, dword [esp+4]
    in      al, dx
    ret

global outportb
outportb:
    mov     eax, dword [esp+8]
    mov     edx, dword [esp+4]
    out     dx, al
    ret

global inportw
inportw:
    mov     edx, dword [esp+4]
    in      ax, dx
    ret

global outportw
outportw:
    mov     eax, dword [esp+8]
    mov     edx, dword [esp+4]
    out     dx, ax
    ret

global inportl
inportl:
    mov     edx, dword [esp+4]
    in      eax, dx
    ret

global outportl
outportl:
    mov     eax, dword [esp+8]
    mov     edx, dword [esp+4]
    out     dx, eax
    ret

global inportsw
inportsw:
    cld                             ; clear direction flag
    mov     edx, dword [esp+12]     ; port to read from
    mov     edi, dword [esp+8]      ; buffer to read into
    mov     ecx, dword [esp+4]      ; number of words to read
    rep insw
    ret

global outportsw
outportsw:
    cld                             ; clear direction flag
    mov     edx, dword [esp+12]     ; port to write to
    mov     edi, dword [esp+8]      ; buffer to read from
    mov     ecx, dword [esp+4]      ; number of words to write
    rep outsw
    ret

global io_delay
io_delay:
    xor     eax, eax
    out     byte 0x80, al
    ret
