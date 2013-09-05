CCHOME = $(HOME)/opt/cross

CC = $(CCHOME)/bin/i386-elf-gcc
NASM = nasm

DEFINES = -DDUNE -DQEMU_DEBUG -g
CFLAGS = $(DEFINES) --std=gnu99 -Wall -Wextra -pedantic -nostdlib -nostdinc \
	-ffreestanding -finline-functions
NFLAGS = -felf
LFLAGS = #-lgcc

KERNDIR = kernel
OBJDIR = obj
ISODIR = isodir

INCLUDES = -I$(KERNDIR)/ -I$(CCHOME)/lib/gcc/i386-elf/4.8.1/include

KERN_SRCS := $(wildcard $(KERNDIR)/*.c) $(wildcard $(KERNDIR)/*.h) $(wildcard $(KERNDIR)/*.asm)
KERN_OBJS := $(addprefix $(OBJDIR)/,\
	start.o main.o io.o gdt.o tss.o idt.o irq.o int.o bget.o mem.o \
	paging.o thread.o blkdev.o initrd.o \
	timer.o kb.o spkr.o rtc.o screen.o string.o print.o util.o \
	elf.o ext2.o fat.o)

KERNEL = kernel.bin
ISO = Dune32.iso
GRUB_CFG = grub.cfg

QEMU = qemu-system-i386
QARGS = -m 32 -initrd README.md -debugcon stdio

.PHONY: all
all: $(KERNEL)

$(KERNEL): $(KERN_OBJS)
	$(CC) $(CFLAGS) -o $@ -T $(KERNDIR)/link.ld $^ $(LFLAGS)

$(OBJDIR)/%.o: $(KERNDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: $(KERNDIR)/%.s
	$(NASM) $(NFLAGS) $< -o $@

$(KERN_OBJS): | $(OBJDIR)
$(OBJDIR):
	test -d $(OBJDIR) || mkdir $(OBJDIR)

$(ISO): $(KERNEL) $(GRUB_CFG)
	cp $< $(ISODIR)/boot/
	cp $(GRUB_CFG) $(ISODIR)/boot/grub/
	grub-mkrescue -o $@ $(ISODIR)

$(ISO): | $(ISODIR)
$(ISODIR):
	mkdir -p $(ISODIR)/boot/grub

.PHONY: iso
iso: $(ISO)

.PHONY: run
run: $(KERNEL)
	$(QEMU) $(QARGS) -kernel $<

tags: $(KERN_SRCS)
	ctags -R -h ".h" --extra=+f

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(KERNEL) $(ISODIR) $(ISO)

.PHONY: help
help:
	@echo "DuneOS Make Help:"
	@echo "  all: make DuneOS kernel"
	@echo "  run: run DuneOS using QEMU"
	@echo "  iso: build a bootable CD-ROM image"
	@echo "  tags: generate ctags"
	@echo "  clean: clean up build files"
