#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <stdint.h>

typedef struct {
  void *renderer;
  void *texture;
} Frontend;

void frontend_init(Frontend *frontend);
void frontend_update(Frontend *frontend, int framebuffer[160 * 144]);
void frontend_draw_tiles(Frontend *frontend, uint8_t *mem);

#endif // __FRONTEND_H__
