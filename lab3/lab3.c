#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include "kbc.h"
#include "keyboard.h"

#include <stdbool.h>
#include <stdint.h>

extern uint8_t scancode[];
extern int scancode_size;
extern bool second_byte;
extern uint8_t data;
extern int timerCounter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {

  uint8_t irq_set;

  if (kbd_subscribe_int(&irq_set))
    return 1;

  int ipc_status;
  message msg;
  while (1) {
    /* Get a request message. */
    int r;
    if ((r = driver_receive(ANY, &msg, &ipc_status))) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();

            process_scancode();

            if (!second_byte) {
              kbd_print_scancode((scancode[scancode_size - 1] & 0x80) == 0, scancode_size, scancode);
              scancode_size = 1;
            }
            if (scancode[scancode_size - 1] == 0x81) {
              if (kbd_unsubscribe_int())
                return 1;
              return 0;
            }
          }
      }
    }
  }

  if (kbd_unsubscribe_int())
    return 1;

  return 0;
}

int(kbd_test_poll)() {

  // if(kbd_disable_int()) return 1;

  while (1) {
    if (kbc_read_output(0x60, &data))
      continue;
    ;

    process_scancode();

    if (!second_byte) {
      kbd_print_scancode((scancode[scancode_size - 1] & 0x80) == 0, scancode_size, scancode);
      scancode_size = 1;
    }
    if (scancode[scancode_size - 1] == 0x81)
      break;
  }

  // if(kbd_restore()) return 1;

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {

  uint8_t kbd_irq_set, timer_irq_set;

  int seconds = 0;

  if (timer_subscribe_int(&timer_irq_set))
    return 1;

  if (kbd_subscribe_int(&kbd_irq_set))
    return 1;

  int ipc_status;
  message msg;
  while (seconds < n) {
    /* Get a request message. */
    int r;
    if ((r = driver_receive(ANY, &msg, &ipc_status))) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & kbd_irq_set) {

            kbc_ih();
            process_scancode();

            seconds = 0;
            timerCounter = 0;

            if (!second_byte) {
              kbd_print_scancode((scancode[scancode_size - 1] & 0x80) == 0, scancode_size, scancode);
              scancode_size = 1;
            }
            if (scancode[scancode_size - 1] == 0x81) {
              if (kbd_unsubscribe_int())
                return 1;
              return 0;
            }
          }
          if (msg.m_notify.interrupts & timer_irq_set) {
            timer_int_handler();
            if (timerCounter % 60 == 0)
              seconds++;
          }
      }
    }
  }

  if (kbd_unsubscribe_int())
    return 1;

  if (timer_unsubscribe_int())
    return 1;

  return 0;
}
