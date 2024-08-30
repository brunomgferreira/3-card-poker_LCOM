#include <lcom/lcf.h>

#include "video.h"
#include "keyboard.h"

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

extern uint8_t scancode[];
extern int scancode_size;
extern bool second_byte;
extern vbe_mode_info_t mode_info;

// Any header files included below this line should have been created by you

int main(int argc, char *argv[])
{
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(wait_ESC)()
{
  uint8_t irq_set;

  if (kbd_subscribe_int(&irq_set))
    return 1;

  int ipc_status, r;
  message msg;
  while (scancode[scancode_size - 1] != 0x81)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)))
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & irq_set)
        {
          kbc_ih();
          process_scancode();
          if (!second_byte)
            scancode_size = 1;
        }
      }
    }
  }

  if (kbd_unsubscribe_int())
    return 1;

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay)
{
  if (set_video_mode(mode))
    return 1;

  sleep(delay);

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color)
{
  if (set_frame_buffer(mode))
    return 1;

  if (set_video_mode(mode))
    return 1;

  uint32_t normalized_color = 0;
  normalize_color(color, &normalized_color);

  if (vg_draw_rectangle(x, y, width, height, normalized_color))
    return 1;

  if (wait_ESC())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step)
{
  if (set_frame_buffer(mode))
    return 1;

  if (set_video_mode(mode))
    return 1;

  unsigned width = mode_info.XResolution / no_rectangles;
  unsigned height = mode_info.YResolution / no_rectangles;

  for (unsigned row = 0; row < no_rectangles; row++)
  {
    for (unsigned col = 0; col < no_rectangles; col++)
    {
      uint32_t color;

      if (mode_info.MemoryModel == 0x06)
      {
        uint32_t R = Red(row, step, first);
        uint32_t G = Green(col, step, first);
        uint32_t B = Blue(row, col, step, first);
        color = direct_mode(R, G, B);
      }
      else
      {
        color = indexed_mode(row, col, step, first, no_rectangles);
      }

      if (vg_draw_rectangle(row * width, col * height, width, height, color))
        return 1;
    }
  }

  if (wait_ESC())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y)
{
  if (set_frame_buffer(0x105))
    return 1;

  if (set_video_mode(0x105))
    return 1;

  xpm_image_t info;
  uint8_t *img = xpm_load(xpm, XPM_INDEXED, &info);

  if (print_xpm(info, img, x, y))
    return 1;

  if (wait_ESC())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate)
{
  uint8_t vertical_direction;
  if (xi == xf && yi < yf)
    // if xi == xf move is vertical
    vertical_direction = 1;
  else if (yi == yf && xi < xf)
    // if yi == yf move is horizontal
    vertical_direction = 0;
  else
    return 1;

  if (set_frame_buffer(0x105))
    return 1;

  if (set_video_mode(0x105))
    return 1;

  uint8_t irq_set_keyboard, irq_set_timer;

  if (kbd_subscribe_int(&irq_set_keyboard))
    return 1;

  if (timer_subscribe_int(&irq_set_timer))
    return 1;

  if (timer_set_frequency(0, fr_rate))
    return 1;

  xpm_image_t info;
  uint8_t *img = xpm_load(xpm, XPM_INDEXED, &info);

  if (print_xpm(info, img, xi, yi))
    return 1;

  int ipc_status, r;
  message msg;
  while ((xi < xf || yi < yf) || (scancode[scancode_size - 1] != 0x81))
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)))
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & irq_set_timer)
        {
          if (clear_xpm(info, xi, yi))
            return 1;

          if (vertical_direction)
          {
            yi += speed;
            if (yi > yf)
              yi = yf;
          }
          else
          {
            xi += speed;
            if (xi > xf)
              xi = xf;
          }

          if (print_xpm(info, img, xi, yi))
            return 1;
        }
        if (msg.m_notify.interrupts & irq_set_keyboard)
        {
          kbc_ih();
          process_scancode();
          if (!second_byte)
            scancode_size = 1;
        }
      }
    }
  }

  if (timer_unsubscribe_int())
    return 1;

  if (kbd_unsubscribe_int())
    return 1;

  if (vg_exit())
    return 1;

  return 0;
}

int(video_test_controller)()
{
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
