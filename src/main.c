#include <stdio.h>

#include "cpu.h"

char ROM[0x200000];
CPU cpu;

char* load_rom(char *filename) {
  FILE *f = fopen(filename, "rb");
  fread(ROM, 1, 0x200000, f);
  fclose(f);

  return ROM;
}

int main() {
  char *rom = load_rom("tetris.gb");

  cpu_init(&cpu, rom);

  while (1) {
    cpu_step(&cpu);
  }

  return 0;
}
