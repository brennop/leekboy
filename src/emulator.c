#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"

#define MAX_CYCLES 70224

static void *load_rom(uint8_t *rom, char *filename) {
  FILE *f = fopen(filename, "rb");
  fread(rom, 1, 0x200000, f);
  fclose(f);

  return rom;
}

void emulator_init(Emulator *emulator, char *filename) {
  load_rom(emulator->rom, filename);

  cpu_init(&emulator->cpu, emulator->rom);
  gpu_init(&emulator->gpu, &emulator->cpu, emulator->cpu.ram);

  emulator->cpu.ram->input = &emulator->input;
}

void emulator_step(Emulator *emulator) {
  int cyclesThisUpdate = 0;

  while (cyclesThisUpdate < MAX_CYCLES) {
    int cycles = cpu_step(&emulator->cpu);
    cyclesThisUpdate += cycles;
    gpu_step(&emulator->gpu, cycles);
  }
}
