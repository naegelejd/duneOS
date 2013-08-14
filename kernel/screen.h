#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>


typedef enum {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    AMBER,
    GRAY,
    DGRAY,
    LBLUE,
    LGREEN,
    LCYAN,
    LRED,
    LMAGENTA,
    LBROWN,
    WHITE
} dune_color_t;

void kset_cursor(unsigned int row, unsigned int col);
char kputc(char ch);
void kcls();
void kset_attr(uint8_t fg, uint8_t bg);

#endif
