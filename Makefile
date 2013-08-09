CCHOME = $(HOME)/opt/cross

CC = $(CCHOME)/bin/i386-elf-gcc
NASM = nasm

CFLAGS = -std=gnu99 -O0 -Wall -Wextra -pedantic -nostdlib -nostdinc -ffreestanding -finline-functions
NFLAGS = -felf
LFLAGS = #-lgcc

KERNDIR = kernel
OBJDIR = obj

INCLUDES = -I$(KERNDIR)/ -I$(CCHOME)/lib/gcc/i386-elf/4.8.1/include

KERN_SRCS := $(wildcard $(KERNDIR)/*.c) $(wildcard $(KERNDIR)/*.asm)
KERN_OBJS := $(addprefix $(OBJDIR)/,start.o kernel.o io.o gdt.o idt.o irq.o \
	paging.o timer.o kb.o spkr.o rtc.o screen.o string.o print.o)

KERNEL = kernel.bin

QEMU = qemu-system-i386
QARGS = -m 512

all: $(KERNEL)

$(KERNEL): $(KERN_OBJS)
	#$(CC) $(CFLAGS) -nostdlib -o $@ -T $(KERNDIR)/link.ld $^
	$(CC) $(CFLAGS) -o $@ -T $(KERNDIR)/link.ld $^ $(LFLAGS)

$(OBJDIR)/%.o: $(KERNDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: $(KERNDIR)/%.s
	$(NASM) $(NFLAGS) $< -o $@

$(KERN_OBJS): | $(OBJDIR)

$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

run: $(KERNEL)
	$(QEMU) $(QARGS) -kernel $<

clean:
	rm -f $(OBJDIR)/*.o $(KERNEL)

distclean:
	rmdir $(OBJDIR)
