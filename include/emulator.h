#ifndef __EMULATOR_H__
#define __EMULATOR_H__

#include "cpu.h"
#include "gpu.h"
#include "ram.h"

typedef struct {
  CPU *cpu;
  GPU *gpu;
  Input input;
} Emulator;

#endif // __EMULATOR_H__

