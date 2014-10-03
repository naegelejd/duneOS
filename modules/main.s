section .text
align 4

global main
main:
    sub esp, 16
    mov     dword [esp+12], esp
    mov     eax, dword [esp+12]
    add     esp, 16
    ret
