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
  uint8_t *rom = load_rom("tetris.gb");

  cpu_init(&cpu, rom);
  gpu_init(&gpu, &cpu, cpu.ram);

  int ticks = 0;
  while (1) {
    /* printf("ticks: %d\n", ticks); */
    int cycles = cpu_step(&cpu);
    ticks += cycles;
    gpu_step(&gpu, cycles);
  }

  return 0;
}
