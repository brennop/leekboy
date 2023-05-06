#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <stdint.h>

void *frontend_init();
void frontend_update(void *renderer, uint8_t framebuffer[160][144]);
void frontend_draw_tiles(void *renderer, uint8_t *mem);

#endif // __FRONTEND_H__
