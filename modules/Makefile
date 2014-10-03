CCHOME ?= $(HOME)/opt/cross

CC = $(CCHOME)/bin/i386-elf-gcc
NASM = nasm

CFLAGS = $(DEFINES) --std=gnu99 -Wall -Wextra -pedantic -nostdlib -nostdinc \
	-ffreestanding -finline-functions
NFLAGS = -felf

INCLUDES = -I$(CCHOME)/lib/gcc/i386-elf/4.9.1/include

MODULE_OBJS := start.o main.o

MODULE = hello.bin

.PHONY: all
all: $(MODULE)

$(MODULE): $(MODULE_OBJS)
	$(CC) $(CFLAGS) -o $@ -T link.ld $^ $(LFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.s
	$(NASM) $(NFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJS) $(MODULE)
