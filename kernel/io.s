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

global io_delay
io_delay:
    xor     eax, eax
    out     byte 0x80, al
    ret
