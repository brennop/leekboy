#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "frontend.h"
#include "gpu.h"

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

  uint8_t *rom = load_rom("tetris.gb");

  cpu_init(&cpu, rom);
  gpu_init(&gpu, &cpu, cpu.ram);

  frontend_init(&frontend);

  const int MAX_CYCLES = 70224;
  while (1) {
    int cycles = 0;

    while (cycles < MAX_CYCLES) {
      cycles += cpu_step(&cpu);
      gpu_step(&gpu, cycles);
    }

    frontend_update(&frontend, gpu.framebuffer, cpu.ram->data + 0x8000);
  }

  return 0;
}
