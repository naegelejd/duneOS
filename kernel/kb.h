#ifndef DUNE_KB_H
#define DUNE_KB_H

enum {
    KBD_STATUS_BUSY = 0x02,
    KBD_DATA_REG = 0x60,
    KBD_CMD_REG = 0x64,
    KBD_STATUS_REG = 0x64,
    KBD_CPU_RESET_PIN = 0xFE
};

void keyboard_install();

typedef uint16_t keycode_t;
keycode_t wait_for_key(void);

#endif /* DUNE_KB_H */
