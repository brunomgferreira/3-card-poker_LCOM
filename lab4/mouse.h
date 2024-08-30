#ifndef __MOUSE_H
#define __MOUSE_H

#include <lcom/lcf.h>

int(mouse_subscribe_int)(uint8_t *bit_no);

int(mouse_unsubscribe_int)();

void(mouse_ih)();

void(process_packet_byte)();

int(mouse_write)(uint8_t command);

int(mouse_gesture)(uint8_t x_len, uint8_t tolerance);

enum States {
  INITIAL,
  FIRST_LINE,
  VERTEX,
  SECOND_LINE,
  COMPLETE,
};

#endif /* __MOUSE_H */
