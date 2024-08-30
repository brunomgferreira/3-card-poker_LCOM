#include <lcom/lcf.h>
#include "video.h"

int(set_video_mode)(uint16_t submode)
{
  reg86_t r86;

  memset(&r86, 0, sizeof(r86));

  r86.intno = 0x10;
  r86.ah = 0x4F;
  r86.al = 0x02;
  r86.bx = submode | BIT(14);

  if (sys_int86(&r86))
  {
    printf("\tvg_exit(): sys_int86() failed \n");
    return 1;
  }

  return 0;
}

int(set_frame_buffer)(uint16_t mode)
{
  memset(&mode_info, 0, sizeof(mode_info));

  if (vbe_get_mode_info(mode, &mode_info))
    return 1;

  unsigned int bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8; // (bits + 7) / 8 is used to always round up
  unsigned int frame_size = mode_info.XResolution * mode_info.YResolution * bytes_per_pixel;

  struct minix_mem_range mr;
  unsigned int vram_base = mode_info.PhysBasePtr; /* VRAM's physical addresss */
  unsigned int vram_size = frame_size;            /* VRAM's size, but you can use the frame-buffer size, instead */

  int r;
  /* Use VBE function 0x01 to initialize vram_base and vram_size */

  mr.mr_base = vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  if ((r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
  {
    printf("sys_privctl (ADD_MEM) failed: %d\n", r);
    return 1;
  }

  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED)
  {
    printf("couldn't map video memory");
    return 1;
  }

  return 0;
}

void(normalize_color)(uint32_t color, uint32_t *normalized_color)
{
  if (mode_info.BitsPerPixel == 32)
  {
    *normalized_color = color;
  }
  else
  {
    *normalized_color = color & (BIT(mode_info.BitsPerPixel) - 1);
    // Make the normalized color have bitsPerPixel size.
    //
    // We do the bitsPerPixel -1 so it becomes all pixels that matters
    // are set to 1.
    //
    // If BitsPerPixel is 8, the bitmask will be 11111111, so the
    // operation will retain only the lowest 8 bits of the color value.
    //
    // If BitsPerPixel is 4, the bitmask will be 00001111, so the
    // operation will retain only the lowest 4 bits of the color value.
  }
}

int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color)
{
  if (x > mode_info.XResolution || y > mode_info.YResolution)
    return 1;

  unsigned bytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

  // Note that the x coordinates start at 0 and increase from left to right
  // Likewise the y coordinates start at 0 and increase from top to bottom
  unsigned int index = (mode_info.XResolution * y + x) * bytesPerPixel;

  if (memcpy(&video_mem[index], &color, bytesPerPixel) == NULL)
  {
    printf("ERROR: vg_draw_pixel()\n");
    return 1;
  }

  return 0;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
  for (unsigned i = 0; i < len; i++)
  {
    if (vg_draw_pixel(x + i, y, color))
      return 1;
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
  for (unsigned i = 0; i < height; i++)
  {
    if (vg_draw_hline(x, y + i, width, color))
    {
      vg_exit();
      return 1;
    }
  }

  return 0;
}

unsigned(indexed_mode)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n)
{
  return (first + (row * n + col) * step) % (1 << mode_info.BitsPerPixel);
}

unsigned(direct_mode)(uint32_t R, uint32_t G, uint32_t B)
{
  return (R << mode_info.RedFieldPosition) | (G << mode_info.GreenFieldPosition) | (B << mode_info.BlueFieldPosition);
}
unsigned(Red)(unsigned col, uint8_t step, uint32_t first)
{
  return (R(first) + col * step) % (1 << mode_info.RedMaskSize);
}

unsigned(Green)(unsigned row, uint8_t step, uint32_t first)
{
  return (G(first) + row * step) % (1 << mode_info.GreenMaskSize);
}

unsigned(Blue)(unsigned row, unsigned col, uint8_t step, uint32_t first)
{
  return (B(first) + (col + row) * step) % (1 << mode_info.BlueMaskSize);
}

unsigned(R)(uint32_t color)
{
  return ((1 << mode_info.RedMaskSize) - 1) & (color >> mode_info.RedFieldPosition);
}

unsigned(G)(uint32_t color)
{
  return ((1 << mode_info.GreenMaskSize) - 1) & (color >> mode_info.GreenFieldPosition);
}

unsigned(B)(uint32_t color)
{
  return ((1 << mode_info.BlueMaskSize) - 1) & (color >> mode_info.BlueFieldPosition);
}

int(print_xpm)(xpm_image_t info, uint8_t *img, uint16_t x, uint16_t y)
{
  for (int h = 0; h < info.height; h++)
  {
    for (int w = 0; w < info.width; w++)
    {
      if (vg_draw_pixel(x + w, y + h, *img))
        return 1;
      img++;
    }
  }

  return 0;
}

int(clear_xpm)(xpm_image_t info, uint16_t x, uint16_t y)
{
  for (int h = 0; h < info.height; h++)
  {
    for (int w = 0; w < info.width; w++)
    {
      if (vg_draw_pixel(x + w, y + h, 0))
        return 1;
    }
  }

  return 0;
}
