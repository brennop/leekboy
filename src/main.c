#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu.h"
#include "gpu.h"

uint8_t ROM[0x200000];
CPU cpu;
GPU gpu;

uint8_t* load_rom(char *filename) {
  FILE *f = fopen(filename, "rb");
  fread(ROM, 1, 0x200000, f);
  fclose(f);

  return ROM;
}

int main() {
  uint8_t *rom = load_rom("individual/01-special.gb");

  cpu_init(&cpu, rom);
  gpu_init(&gpu, &cpu, cpu.ram);

  while (1) {
    int cycles = cpu_step(&cpu);
    gpu_step(&gpu, cycles);
  }

  return 0;
}
