; vim:syntax=nasm
extern main

section .text
start:
    call main
    ;jmp $
    ret
