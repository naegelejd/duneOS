DuneOS
======

A simple UNIX-like operating system

Build Dependencies
------------------
You'll need an i386 C-compiler, linker, etc.

The `build-cross-compiler.sh` script should help you obtain and build GNU binutils and GCC for i386.

You'll also need `GNU make` and `nasm`.

Emulator
--------
To try DuneOS, obtain `QEMU` (specifically `qemu-system-i386`) then:

    make run

CD-ROM
------
To make a bootable CD-ROM image, you'll need the tools packaged with GRUB, specifically
`grub-mkrescue` and it's primary dependency `xorriso` (not available on Mac OS X).

    make iso

This generates an image called `Dune32.iso`, which will run on a VMWare or VirtualBox VM.

References
----------
GeekOS: http://geekos.sourceforge.net/

OS Dev.org: http://wiki.osdev.org/Main_Page

Bran's Kernel Development Tutorial: http://www.osdever.net/bkerndev/Docs/title.htm

JamesM's Kernel Development Tutorials: http://www.jamesmolloy.co.uk/tutorial_html/index.html

Writing a Simple Operating System from Scratch (PDF): http://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

Building GCC Cross-Compiler: http://wiki.osdev.org/GCC_Cross-Compiler
