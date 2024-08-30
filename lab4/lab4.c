// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "i8042.h"
#include "mouse.h"
#include "time.h"

extern struct packet mPacket;
extern int byteIndex;
extern int timerCounter;
extern enum States current_state;

int main(int argc, char *argv[])
{
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt)
{
  uint8_t irq_set;

  if (mouse_write(ENABLE_DATA_REPORTING))
    return 1;

  // call int(mouse_subscribe_int)(uint8_t *bit_no)
  if (mouse_subscribe_int(&irq_set))
    return 1;

  int ipc_status, r;
  message msg;

  while (cnt)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)))
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    // Check if message is a notification
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & irq_set)
        {
          // Call interrupt handler
          
          mouse_ih();
          // mouse_print_packet(&mPacket);

          process_packet_byte();

          if (byteIndex == 3)
          {
            // call void mouse_print_packet(struct packet *pp)
            mouse_print_packet(&mPacket);
            byteIndex = 0;
            cnt--;
          }
        }
      }
    }
  }

  if (mouse_unsubscribe_int())
    return 1;

  if (mouse_write(DISABLE_DATA_REPORTING))
    return 1;

  return 0;
}

int(mouse_test_async)(uint8_t idle_time)
{
  uint16_t timer_frequency = sys_hz();
  uint8_t mouse_irq_set, timer_irq_set;

  int ipc_status, r;
  message msg;

  int seconds = 0;

  if (timer_subscribe_int(&timer_irq_set))
    return 1;

  if (mouse_write(ENABLE_DATA_REPORTING))
    return 1;

  // call int(mouse_subscribe_int)(uint8_t *bit_no)
  if (mouse_subscribe_int(&mouse_irq_set))
    return 1;

  while (seconds < idle_time)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    // Check if message is a notification
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & mouse_irq_set)
        {
          // Call interrupt handler
          mouse_ih();

          process_packet_byte();

          seconds = 0;
          timerCounter = 0;

          if (byteIndex == 3)
          {
            // call void mouse_print_packet(struct packet *pp)
            mouse_print_packet(&mPacket);
            byteIndex = 0;
          }
        }
        if (msg.m_notify.interrupts & timer_irq_set)
        {
          timer_int_handler();
          if (timerCounter % timer_frequency == 0)
            seconds++;
        }
      }
    }
  }

  if (mouse_unsubscribe_int())
    return 1;

  if (mouse_write(DISABLE_DATA_REPORTING))
    return 1;  

  if (timer_unsubscribe_int())
    return 1;

  return 0;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance)
{
  uint8_t irq_set;

  if (mouse_write(ENABLE_DATA_REPORTING))
    return 1;

  // call int(mouse_subscribe_int)(uint8_t *bit_no)
  if (mouse_subscribe_int(&irq_set))
    return 1;

  int ipc_status, r;
  message msg;

  while (current_state != COMPLETE)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)))
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    // Check if message is a notification
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & irq_set)
        {
          // Call interrupt handler
          mouse_ih();

          process_packet_byte();

          if (byteIndex == 3)
          {
            // call void mouse_print_packet(struct packet *pp)
            mouse_gesture(x_len, tolerance);
            mouse_print_packet(&mPacket);
            byteIndex = 0;
          }
        }
      }
    }
  }

  if (mouse_unsubscribe_int())
    return 1;

  if (mouse_write(DISABLE_DATA_REPORTING))
    return 1;

  return 0;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt)
{
  /* This year you need not implement this. */
  printf("%s(%u, %u): under construction\n", __func__, period, cnt);
  return 1;
}
