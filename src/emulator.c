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

  ram_init(&emulator->ram, &emulator->input, emulator->rom);
  cpu_init(&emulator->cpu, &emulator->ram);
  gpu_init(&emulator->gpu, &emulator->cpu, emulator->cpu.ram);
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
  uint8_t div = ram_get(emulator->cpu.ram, MEM_DIV);
  uint8_t tima = ram_get(emulator->cpu.ram, MEM_TIMA);
  uint8_t tma = ram_get(emulator->cpu.ram, MEM_TMA);
  
  emulator->div += cycles;
  if (emulator->div >= 256) {
    emulator->div = 0;
    ram_set(emulator->cpu.ram, MEM_DIV, div + 1);
  }

  if (timer_attrs & 0x04) {
    emulator->tima += cycles;

    uint8_t freq = timer_attrs & 0x03;
    uint16_t clock_speed = freqs[freq];

    if (emulator->tima >= clock_speed) {
      emulator->tima = 0;
      ram_set(emulator->cpu.ram, MEM_TIMA, tima + 1);

      // if TIMA overflows, reset to TMA and trigger interrupt
      if (tima == 0xFF) {
        ram_set(emulator->cpu.ram, MEM_TIMA, tma);
        cpu_interrupt(&emulator->cpu, INT_TIMER);
      }
    }
  }

}
