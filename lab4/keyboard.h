#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <lcom/lcf.h>

int(kbd_subscribe_int)(uint8_t *bit_no);
int(kbd_unsubscribe_int)();
void(kbc_ih)();
void(process_scancode)();
int(kbd_restore)();
int(kbd_disable_int)();

#endif /* __KEYBOARD_H */
