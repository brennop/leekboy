#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "cpu.h"
#include "gpu.h"
#include "ram.h"

#define CLOCKSPEED 4194304


typedef struct {
  CPU cpu;
  GPU gpu;
  RAM ram;

  Input input;
  uint8_t rom[0x200000];

  // timer
  int div;
  int tima;
} Emulator;

void emulator_init(Emulator *emulator, char *filename);
void emulator_step(Emulator *emulator);
void emulator_update_timers(Emulator *emulator, int cycles);

#endif // __EMULATOR_H__

