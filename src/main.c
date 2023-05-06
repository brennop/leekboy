#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu.h"
#include "gpu.h"
#include "frontend.h"

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

  void* renderer = frontend_init();

  const int MAX_CYCLES = 70224;
  while (1) {
    int cycles = 0;

    while (cycles < MAX_CYCLES) {
      cycles += cpu_step(&cpu);
      gpu_step(&gpu, cycles);

    }

    frontend_update(renderer, gpu.framebuffer);
    frontend_draw_tiles(renderer, cpu.ram->data + 0x8000);
  }

  return 0;
}
