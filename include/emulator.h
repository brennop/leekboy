#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "cpu.h"
#include "gpu.h"
#include "ram.h"

typedef struct {
  CPU cpu;
  GPU gpu;
  Input input;
  uint8_t rom[0x200000];
} Emulator;

void emulator_init(Emulator *emulator, char *filename);
void emulator_step(Emulator *emulator);

#endif // __EMULATOR_H__

