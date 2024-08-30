#ifndef __VIDEO_H
#define __VIDEO_H

#include <lcom/lcf.h>

vbe_mode_info_t mode_info;
static char *video_mem;

int(set_video_mode)(uint16_t submode);

int(set_frame_buffer)(uint16_t mode);

void(normalize_color)(uint32_t color, uint32_t *normalized_color);
int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

unsigned(indexed_mode)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n);

unsigned(direct_mode)(uint32_t R, uint32_t G, uint32_t B);
unsigned(Red)(unsigned col, uint8_t step, uint32_t first);
unsigned(Green)(unsigned row, uint8_t step, uint32_t first);
unsigned(Blue)(unsigned row, unsigned col, uint8_t step, uint32_t first);
unsigned(R)(uint32_t color);
unsigned(G)(uint32_t color);
unsigned(B)(uint32_t color);

int(print_xpm)(xpm_image_t info, uint8_t *img, uint16_t x, uint16_t y);
int(clear_xpm)(xpm_image_t info, uint16_t x, uint16_t y);

#endif /* __VIDEO_H */
