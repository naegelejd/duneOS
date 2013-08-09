#include <system.h>
#include <io.h>
#include <screen.h>
#include "kb.h"

/* US Keyboard Layout lookup table */
unsigned char kdbus[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
    '9', '0', '-', '=', '\b',   /* Backspace */
    '\t',                       /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       /* Enter key */
    0,                  /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
    '\'', '`',   0,             /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',                 /* 49 */
    'm', ',', '.', '/',   0,                            /* Right shift */
    '*',
    0,  /* Alt */
    ' ', /* Space bar */
    1,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};


static void toggle_light(unsigned int light_code)
{
    while (inportb(KBD_STATUS_REG) & KBD_STATUS_BUSY)
        ;
    outportb(KBD_DATA_REG, 0xED);
    outportb(KBD_DATA_REG, light_code & 0xF);
}

/*
static void toggle_scroll_lock_light(void)
{
    toggle_light(0x0);
}
static void toggle_num_lock_light(void)
{
    toggle_light(0x1);
}
*/

static void toggle_caps_lock_light(void)
{
    toggle_light(0x2);
}


void keyboard_handler(struct regs *r)
{
    (void)r;    /* prevent 'unused parameter' warning */
    /* read from keyboard data register */
    unsigned char data = inportb(KBD_DATA_REG);
    unsigned char scancode = data & 0x7F;

    if (data & 0x80) {
        /* key released */
        if (scancode == 1) {
            toggle_caps_lock_light();
        }
    } else {
        /* key pressed */
        k_putchar(kdbus[scancode], 0);
    }
}

/* installs keyboard_handler into IRQ1 */
void keyboard_install()
{
    irq_install_handler(1, keyboard_handler);
}
