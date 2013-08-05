CCHOME = $(HOME)/opt/cross

CC = $(CCHOME)/bin/i386-elf-gcc
LD = $(CCHOME)/bin/i386-elf-ld
NASM = nasm

CFLAGS = -std=c99 -O0 -Wall -Wextra -pedantic -nostdinc -ffreestanding -finline-functions
NFLAGS = -f elf

BOOTDIR = boot
KERNDIR = kernel

INCLUDES = -I$(KERNDIR)/ -I$(CCHOME)/lib/gcc/i386-elf/4.8.1/include

BOOTSECT = $(BOOTDIR)/sector.asm
KERN_SRCS := $(wildcard $(KERNDIR)/*.c) $(wildcard $(KERNDIR)/*.asm)
KERN_OBJS := start.o kernel.o io.o screen.o string.o gdt.o idt.o irq.o timer.o kb.o spkr.o rtc.o

IMAGE = duneOS.img

all: $(IMAGE)

$(IMAGE): loader.bin kernel.bin
	cat $^ > $@

loader.bin: $(BOOTSECT)
	$(NASM) $< -f bin -o $@

kernel.bin: $(KERN_OBJS)
	$(LD) -o $@ -T $(KERNDIR)/link.ld $^ --oformat binary

%.o: $(KERNDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: $(KERNDIR)/%.asm
	$(NASM) $(NFLAGS) $< -o $@

run: $(IMAGE)
	qemu-system-i386 --soundhw pcspk -boot order=adc -fda $<

clean:
	rm -f *.bin *.o $(IMAGE)
