#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>

typedef struct {
  uint8_t *memory;

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
} CPU;

void cpu_init(CPU *cpu, uint8_t *rom);
int cpu_step(CPU *cpu);

#endif // __CPU_H__