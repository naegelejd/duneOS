duneOS
======

A simple UNIX-like operating system

Build Dependencies
------------------
You'll need an i386 C-compiler, linker, etc.

The `build-cross-compiler.sh` script should help you obtain and build GNU binutils and GCC for i386.

You'll also need `GNU make` and `nasm`.

    make

Run
---
Obtain `qemu` then:

    make run


References
----------
Bran's Kernel Development Tutorial: http://www.osdever.net/bkerndev/Docs/title.htm

Writing a Simple Operating System from Scratch (PDF): http://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

Building GCC Cross-Compiler: http://wiki.osdev.org/GCC_Cross-Compiler
