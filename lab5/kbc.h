#ifndef __KBC_H
#define __KBC_H

#include <lcom/lcf.h>
#include "i8042.h"

int(kbc_write_command)(uint8_t port, uint8_t command);

int(kbc_read_output)(uint8_t port, uint8_t *cmd, uint8_t mouse);

#endif /* __KBC_H */
