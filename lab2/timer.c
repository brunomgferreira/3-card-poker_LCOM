#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"


int counter = 0;
int hook_id = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if(timer < 0 || timer > 2) return 1;
  if(freq < 19 || freq > TIMER_FREQ) return 1;
  
  uint16_t c = TIMER_FREQ / freq;
  uint8_t st;
  uint8_t msb = 0;
  uint8_t lsb = 0;

  if(timer_get_conf(timer, &st)) return 1;
  if(sys_outb(TIMER_CTRL, (st & 0xF) | TIMER_LSB_MSB | BIT(timer + 5))) return 1;

  if(util_get_LSB(c, &lsb) || util_get_MSB(c, &msb)) return 1;

  if(sys_outb(TIMER_0 + timer, lsb)) return 1;
  if(sys_outb(TIMER_0 + timer, msb)) return 1;
  
  return 0;
  
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  if(bit_no == NULL) return 1;
  *bit_no = hook_id;
  if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0) return 1;
  return 0;
}

int (timer_unsubscribe_int)() {
  if(sys_irqrmpolicy(&hook_id) != 0) return 1;
  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(timer > 2) return 1;
  uint8_t rb = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  if(sys_outb(TIMER_CTRL, rb)) return 1;
  if(util_sys_inb(TIMER_0 + timer, st)) return 1;
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  union timer_status_field_val aux;
  switch (field) {
  case (tsf_all):
    aux.byte = st;
    break;

  case (tsf_initial):
    st = (st >> 4) & 3;
    if (st == 1) aux.in_mode = LSB_only;
    else if (st == 2) aux.in_mode = MSB_only;
    else if (st == 3) aux.in_mode = MSB_after_LSB;
    else aux.in_mode = INVAL_val;
    break;

  case (tsf_mode):
    st = (st >> 1) & 7;
    if (st == 2 || st == 6) aux.count_mode = 2;
    else if (st == 3 || st == 7) aux.count_mode = 3;
    else aux.count_mode = st;
    break;

  case (tsf_base):
    aux.bcd = st & 1;
    break;

  default:
    break;
  }
  
  if (timer_print_config(timer, field, aux) != 0) return 1;
  return 0;
}
