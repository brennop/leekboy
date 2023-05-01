#ifndef __CPU_H__
#define __CPU_H__

#include "ram.h"
#include <stdint.h>

#define IF 0xFF0F
#define IE 0xFFFF

#define INT_VBLANK 0x01
#define INT_LCDSTAT 0x02
#define INT_TIMER 0x04
#define INT_SERIAL 0x08
#define INT_JOYPAD 0x10

typedef struct {
  RAM *ram;

  union {
    struct {
      uint8_t f;
      uint8_t a;
    };
    uint16_t af;
  };

  union {
    struct {
      uint8_t c;
      uint8_t b;
    };
    uint16_t bc;
  };

  union {
    struct {
      uint8_t e;
      uint8_t d;
    };
    uint16_t de;
  };

  union {
    struct {
      uint8_t l;
      uint8_t h;
    };
    uint16_t hl;
  };

  uint16_t sp;
  uint16_t pc;

  uint8_t ime;
} CPU;

void cpu_init(CPU *cpu, uint8_t *rom);
int cpu_step(CPU *cpu);
void cpu_interrupt(CPU *cpu, uint8_t interrupt);

extern void cpu_memory_set(CPU *cpu, uint16_t address, uint8_t value);
extern uint8_t cpu_memory_get(CPU *cpu, uint16_t address);

#endif // __CPU_H__
