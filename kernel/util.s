; vim:syntax=nasm

section .text
align 4

global get_esp
get_esp:
    sub     esp, 16
    mov     dword [esp+12], esp
    mov     eax, dword [esp+12]
    add     esp, 16
    ret

global khalt
khalt:
    cli
    hlt
    ret

; Attempt to triple-fault (doesn't really work)
global kreboot
kreboot:
    ;lidt no_idt
    lidt [no_idt]
    ret

section .data
align 4
;global  no_idt

no_idt:
    ;resw 2
    times 6 db 0
