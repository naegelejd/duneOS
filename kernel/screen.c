#include "screen.h"
#include "string.h"
#include "io.h"

#define VIDEO_ADDR      0xb8000
#define VIDEO_ROWS      25
#define VIDEO_COLS      80

/* Screen device I/O ports */
#define REG_SCREEN_CTRL     0x3D4
#define REG_SCREEN_DATA     0x3D5
#define REG_CURSOR_LOW      0x0F
#define REG_CURSOR_HIGH     0x0E

#define WHITE_ON_BLACK      0x0F
#define WHITE_ON_RED        0x4F

/* global attribute for all chars printed to console */
static uint8_t g_attr = WHITE_ON_BLACK;


static unsigned int get_cursor_offset();
static void set_cursor_offset(unsigned int offset);
static unsigned int screen_offset(int row, int col);
static unsigned int scroll_screen (unsigned int offset);


char kputc(char ch)
{
    char* vidmem = (char *) VIDEO_ADDR;

    unsigned int offset = get_cursor_offset();

    if (ch == '\n') {
        int row = offset / (VIDEO_COLS * 2);
        offset = screen_offset(row + 1, 0);
    } else {
        vidmem[offset] = ch;
        vidmem[offset+1] = g_attr;
        offset += 2;
    }

    offset = scroll_screen(offset);

    set_cursor_offset(offset);

    return ch;
}

void kset_cursor(unsigned int row, unsigned int col)
{
    unsigned int offset = screen_offset(row, col);
    set_cursor_offset(offset);
}

void kcls() // clear the entire text screen
{
    char *vidmem = (char *) VIDEO_ADDR;
    unsigned int i = 0;
    while (i < ((VIDEO_COLS * VIDEO_ROWS) * 2)) {
        /* low byte = ASCII char, high byte = style */
        vidmem[i++] = ' ';
        vidmem[i++] = WHITE_ON_BLACK;
    }
    set_cursor_offset(0);
}

void kset_attr(uint8_t fg, uint8_t bg)
{
    g_attr = bg << 8 | fg;
}


static unsigned int get_cursor_offset()
{
    unsigned int offset = 0;
    outportb(REG_SCREEN_CTRL, REG_CURSOR_HIGH);     /* high byte of cursor offset */
    offset = inportb(REG_SCREEN_DATA) << 8;
    outportb(REG_SCREEN_CTRL, REG_CURSOR_LOW);     /* low  byte of cursor offset */
    offset += inportb(REG_SCREEN_DATA);

    /* return offset * sizeof character cell (2) */
    return offset * 2;
}

static void set_cursor_offset(unsigned int offset)
{
    offset /= 2;    /* convert from cell offset to char offset */
    outportb(REG_SCREEN_CTRL, REG_CURSOR_HIGH);
    outportb(REG_SCREEN_DATA, (uint8_t)(offset >> 8));
    outportb(REG_SCREEN_CTRL, REG_CURSOR_LOW);
    outportb(REG_SCREEN_DATA, (uint8_t)(offset));
}

static unsigned int screen_offset(int row, int col)
{
    unsigned int offset = 0;
    offset = (row * VIDEO_COLS + col) * 2;
    return offset;
}

static unsigned int scroll_screen (unsigned int offset)
{
    if (offset < VIDEO_ROWS * VIDEO_COLS * 2) {
        return offset;
    }

    /* copy each row to the row above it */
    unsigned int i = 1;
    for (i = 1; i < VIDEO_ROWS; i++) {
        memcpy(
                (char*)(VIDEO_ADDR + screen_offset(i-1, 0)),
                (char*)(VIDEO_ADDR + screen_offset(i, 0)),
                VIDEO_COLS * 2);
    }

    /* zero out last line */
    char* last_line = (char*)(VIDEO_ADDR + screen_offset(VIDEO_ROWS - 1, 0));
    for (i = 0; i < VIDEO_COLS * 2; i++) {
        last_line[i] = 0;
    }

    /* move cursor offset up 1 row */
    offset -= 2 * VIDEO_COLS;

    return offset;
}
