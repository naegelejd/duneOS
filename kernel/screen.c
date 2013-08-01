#include "screen.h"
#include "string.h"
#include "io.h"

#define VIDEO_ADDR      0xb8000
#define VIDEO_ROWS      25
#define VIDEO_COLS      80

#define WHITE_ON_BLACK      0x0f

/* Screen device I/O ports */
#define REG_SCREEN_CTRL     0x3D4
#define REG_SCREEN_DATA     0x3D5

/*
    0   BLACK       8   DARK GREY
    1   BLUE        9   LIGHT BLUE
    2   GREEN       10  LIGHT GREEN
    3   CYAN        11  LIGHT CYAN
    4   RED         12  LIGHT RED
    5   MAGENTA     13  LIGHT MAGENTA
    6   BROWN       14  LIGHT BROWN
    7   LIGHT GREY  15  WHITE
*/

static unsigned int k_get_screen_offset(int col, int row);
static unsigned int k_scroll_screen (unsigned int offset);

static unsigned int k_get_screen_offset(int col, int row)
{
    unsigned int offset = 0;
    offset = (row * VIDEO_COLS + col) * 2;
    return offset;
}

static unsigned int k_scroll_screen (unsigned int offset)
{
    if (offset < VIDEO_ROWS * VIDEO_COLS * 2) {
        return offset;
    }

    /* copy each row to the row above it */
    unsigned int i = 1;
    for (i = 1; i < VIDEO_ROWS; i++) {
        memcpy(
                (char*)(VIDEO_ADDR + k_get_screen_offset(0, i-1)),
                (char*)(VIDEO_ADDR + k_get_screen_offset(0, i)),
                VIDEO_COLS * 2);
    }

    /* zero out last line */
    char* last_line = (char*)(VIDEO_ADDR + k_get_screen_offset(0, VIDEO_ROWS - 1));
    for (i = 0; i < VIDEO_COLS * 2; i++) {
        last_line[i] = 0;
    }

    /* move cursor offset up 1 row */
    offset -= 2 * VIDEO_COLS;

    return offset;
}


int k_get_cursor ()
{
    int offset = 0;
    outportb(REG_SCREEN_CTRL, 14);     /* high byte of cursor offset */
    offset = inportb(REG_SCREEN_DATA) << 8;
    outportb(REG_SCREEN_CTRL, 15);     /* low  byte of cursor offset */
    offset += inportb(REG_SCREEN_DATA);

    /* return offset * sizeof character cell (2) */
    return offset * 2;
}

void k_set_cursor (unsigned int offset)
{
    offset /= 2;    /* convert from cell offset to char offset */
    outportb(REG_SCREEN_CTRL, 14);
    outportb(REG_SCREEN_DATA, (uint8_t)(offset >> 8));
    outportb(REG_SCREEN_CTRL, 15);
    outportb(REG_SCREEN_DATA, (uint8_t)(offset));
}

void k_clear_screen() // clear the entire text screen
{
    char *vidmem = (char *) VIDEO_ADDR;
    unsigned int i = 0;
    while (i < ((VIDEO_COLS * VIDEO_ROWS) * 2)) {
        /* low byte = ASCII char, high byte = style */
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE_ON_BLACK;
    }
    k_set_cursor(0);
}

char k_putchar(char ch, uint8_t attr)
{
    char* vidmem = (char *) VIDEO_ADDR;
    if (!attr) {
        attr = WHITE_ON_BLACK;
    }

    int offset = k_get_cursor();

    if (ch == '\n') {
        int row = offset / (VIDEO_COLS * 2);
        offset = k_get_screen_offset(0, row + 1);
    } else {
        vidmem[offset] = ch;
        vidmem[offset+1] = attr;
        offset += 2;
    }

    offset = k_scroll_screen(offset);

    k_set_cursor(offset);

    return ch;
}

void k_puts(char *msg)
{
    char c;
    while ((c = *msg++) != 0) {
        k_putchar(c, WHITE_ON_BLACK);
    }
}
