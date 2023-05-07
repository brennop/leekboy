#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "frontend.h"
#include "gpu.h"
#include "emulator.h"

uint8_t ROM[0x200000];


int main() {
  Frontend frontend;
  Emulator emulator;

  frontend_init(&frontend);
  emulator_init(&emulator, "tetris.gb");

  frontend_run(&frontend, &emulator);

  return 0;
}
