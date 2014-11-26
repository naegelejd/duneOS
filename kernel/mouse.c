#include "irq.h"
#include "io.h"
#include "assert.h"

/* Reference: http://wiki.osdev.org/Mouse_Input */

bool g_mouse_left, g_mouse_right, g_mouse_middle;
int8_t g_mouse_dx, g_mouse_dy;

enum {
    MOUSE_DATA_REG = 0x60,
    MOUSE_CMD_REG = 0x64,
};

static void mouse_handler(struct regs *r)
{
    (void)r;
    /* DEBUGF("Mouse: %02d, %02d\n", g_mouse_x, g_mouse_y); */

    static uint8_t mouse_cycle;
    static int8_t mouse_byte[3];
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = inportb(MOUSE_DATA_REG);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = inportb(MOUSE_DATA_REG);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = inportb(MOUSE_DATA_REG);
            g_mouse_left = (mouse_byte[0] & 0x01) != 0;
            g_mouse_right = (mouse_byte[0] & 0x02) != 0;
            g_mouse_middle = (mouse_byte[0] & 0x04) != 0;
            g_mouse_dx = mouse_byte[1];
            g_mouse_dy = mouse_byte[2];
            mouse_cycle = 0;
            break;
        default:
            KASSERT(false);
    }
}

static void mouse_wait(int type)
{
  int timeout = 100000;

  if (type == 0) {
    /* data */
    while (timeout--) {
      if ((inportb(MOUSE_CMD_REG) & 1) == 1) {
        return;
      }
    }
    return;
  }
  else {
    /* signal */
    while (timeout--) {
      if ((inportb(MOUSE_CMD_REG) & 2) == 0) {
        return;
      }
    }
    return;
  }
}

static void mouse_write(uint8_t b)
{
    mouse_wait(1);
    /* tell the mouse we are sending a command */
    outportb(MOUSE_CMD_REG, 0xD4);
    /* wait for the final part */
    mouse_wait(1);
    /* finally write */
    outportb(MOUSE_DATA_REG, b);
}

void mouse_install(void)
{
    uint8_t status;

    mouse_wait(1);
    outportb(MOUSE_CMD_REG, 0xA8); /* enable auxiliary mouse device */

    /* enable interrupts */
    mouse_wait(1);
    outportb(MOUSE_CMD_REG, 0x20);  /* Get Compaq status byte */
    mouse_wait(0);
    status = inportb(MOUSE_DATA_REG);   /* read status byte */
    status |= 0x02;     /* enable IRQ 12 */
    status &= ~0x20;    /* disable mouse clock */
    mouse_wait(1);
    outportb(MOUSE_CMD_REG, 0x60);  /* set Compaq status byte */
    mouse_wait(1);
    outportb(MOUSE_DATA_REG, status);   /* send status byte */

    /* use mouse default settings:
     * packet rate: 100 packets/second
     * resolution: 4 pixels/mm */
    mouse_write(0xF6);
    mouse_wait(0);
    inportb(MOUSE_DATA_REG); /* acknowledge */

    /* enable mouse packet streaming */
    mouse_write(0xF4);
    mouse_wait(0);
    inportb(MOUSE_DATA_REG); /* acknowledge */

    irq_install_handler(IRQ_MOUSE, mouse_handler);
    enable_irq(IRQ_MOUSE);
}
