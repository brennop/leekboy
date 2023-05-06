#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <stdint.h>
#include <cpu.h>

typedef struct Input {
  uint8_t up;
  uint8_t down;
  uint8_t left;
  uint8_t right;
  uint8_t a;
  uint8_t b;
  uint8_t start;
  uint8_t select;
} Input;


typedef struct {
  void *renderer;
  void *texture;
  Input input;
} Frontend;

void frontend_init(Frontend *frontend);
void frontend_update(Frontend *frontend, int framebuffer[160 * 144], CPU *cpu);
void frontend_draw_tiles(Frontend *frontend, uint8_t *mem);

#endif // __FRONTEND_H__
