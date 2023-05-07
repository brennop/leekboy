#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <stdint.h>
#include "emulator.h"

typedef struct {
  void *renderer;
  void *texture;
} Frontend;

void frontend_init(Frontend *frontend);
void frontend_update(Frontend *frontend, Emulator *emulator);

#endif // __FRONTEND_H__
