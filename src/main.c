#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cpu.h"

uint8_t ROM[0x200000];
CPU cpu;

uint8_t* load_rom(char *filename) {
  FILE *f = fopen(filename, "rb");
  fread(ROM, 1, 0x200000, f);
  fclose(f);

  return ROM;
}

int main() {
  uint8_t *rom = load_rom("tetris.gb");

  cpu_init(&cpu, rom);

  while (1) {
    cpu_step(&cpu);
  }

  return 0;
}
