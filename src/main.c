#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"
#include "frontend.h"
#include "gpu.h"

int main(int argc, char **argv) {
  Frontend frontend;
  Emulator emulator;

  frontend_init(&frontend);
  emulator_init(&emulator, argv[1]);

  frontend_run(&frontend, &emulator);

  return 0;
}
