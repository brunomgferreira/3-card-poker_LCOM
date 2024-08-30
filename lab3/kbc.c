#include <lcom/lcf.h>
#include "kbc.h"
#include "i8042.h"

int(kbc_write_command)(uint8_t port, uint8_t cmd) {

  int tries = 10;
  uint8_t stat;

  while (tries > 0) {
    if (util_sys_inb(KBC_STAT_REG, &stat))
      return 1;
    /* loop while 8042 input buffer is not empty */
    if ((stat & BIT(1)) == 0) {
      if (sys_outb(KBC_CTRL_REG, cmd))
        return 1;
      return 0;
    }
    tries--;
    tickdelay(micros_to_ticks(20000)); // e.g. tickdelay()
  }
  return 1;
}

int(kbc_read_output)(uint8_t port, uint8_t *cmd) {
  int tries = 10;
  uint8_t stat;

  while (tries > 0) {
    if (util_sys_inb(KBC_STAT_REG, &stat))
      return 1;
    if (stat & BIT(0)) {

      if (util_sys_inb(port, cmd))
        return 1;

      if ((stat & (BIT(7) | BIT(6))) || (stat & BIT(5)))
        return 1;

      return 0;
    }
    tries--;
    tickdelay(micros_to_ticks(20000));
  }
  return 1;
}
