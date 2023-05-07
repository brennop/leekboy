#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"

#define MAX_CYCLES 70224
const uint16_t freqs[] = { 1024, 16, 64, 256 };

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
    emulator_update_timers(emulator, cycles);
    gpu_step(&emulator->gpu, cycles);
  }
}

void emulator_update_timers(Emulator *emulator, int cycles) {
  uint8_t timer_attrs = ram_get(emulator->cpu.ram, MEM_TAC);
  
  emulator->div += cycles;
  if (emulator->div >= 256) {
    emulator->div = 0;
    emulator->cpu.ram->data[MEM_DIV]++;
  }

  if (timer_attrs & 0x04) {
    emulator->tima += cycles;

    uint8_t freq = timer_attrs & 0x03;
    uint16_t clock_speed = freqs[freq];

    if (emulator->tima >= clock_speed) {
      emulator->tima = 0;
      emulator->cpu.ram->data[MEM_TIMA]++;

      // if TIMA overflows, reset to TMA and trigger interrupt
      if (emulator->cpu.ram->data[MEM_TIMA] == 0) {
        emulator->cpu.ram->data[MEM_TIMA] = emulator->cpu.ram->data[MEM_TMA];
        cpu_interrupt(&emulator->cpu, INT_TIMER);
      }
    }
  }

}
