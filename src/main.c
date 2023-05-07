#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "frontend.h"
#include "gpu.h"
#include "emulator.h"

uint8_t ROM[0x200000];

uint8_t *load_rom(char *filename) {
  FILE *f = fopen(filename, "rb");
  fread(ROM, 1, 0x200000, f);
  fclose(f);

  return ROM;
}

int main() {
  CPU cpu;
  GPU gpu;
  Frontend frontend;

  // TODO: move to emulator_init
  Emulator emulator;
  emulator.cpu = &cpu;
  emulator.gpu = &gpu;

  uint8_t *rom = load_rom("tetris.gb");

  cpu_init(&cpu, rom);
  gpu_init(&gpu, &cpu, cpu.ram);

  cpu.ram->input = &emulator.input;

  frontend_init(&frontend);

  const int MAX_CYCLES = 69905;
  while (1) {
    int cyclesThisUpdate = 0;

    while (cyclesThisUpdate < MAX_CYCLES) {
      int cycles = cpu_step(&cpu);
      cyclesThisUpdate += cycles;
      gpu_step(&gpu, cycles);
    }

    frontend_update(&frontend, &emulator);
  }

  return 0;
}
