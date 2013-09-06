CC=gcc
CFLAGS = -DDUNE -g --std=gnu99 -Wall -Wextra
INCLUDES = -I..
OBJS := ext2fmt.o

all: ext2fmt

ext2fmt: ext2fmt.o
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean:
	rm -rf $(OBJS)
