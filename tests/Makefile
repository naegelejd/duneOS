CCHOME = $(HOME)/opt/cross

CC = $(CCHOME)/bin/i386-elf-gcc
DEFINES = -DDUNE -DQEMU_DEBUG -g
CFLAGS = $(DEFINES) --std=gnu99 -Wall -Wextra -pedantic -nostdlib -nostdinc \
	-ffreestanding -finline-functions

INCLUDE = -I../kernel/ -I$(CCHOME)/lib/gcc/i386-elf/4.8.1/include

TESTS := strings

all:
	$(foreach var, $(TESTS), \
		$(CC) $(CFLAGS) $(INCLUDE) $(var).c -o $(var))
