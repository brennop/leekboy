#include "cpu.h"
#include "instructions.h"
#include "ram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ZEROF (cpu->f & 0x80) == 0x80
#define SUBF (cpu->f & 0x40) == 0x40
#define HALFF (cpu->f & 0x20) == 0x20
#define CARRYF (cpu->f & 0x10) == 0x10

#define NN ram_get(cpu->ram, cpu->pc - 1)
#define NNN ram_get(cpu->ram, cpu->pc - 1) << 8 | ram_get(cpu->ram, cpu->pc - 2)

#define CASE4_16(x) case x: case x + 16: case x + 32: case x + 48:
#define CASE8_8(x) case x: case x + 8: case x + 16: case x + 24: case x + 32: case x + 40: case x + 48: case x + 56:

void cpu_ram_set(CPU *cpu, uint16_t address, uint8_t value) {
  ram_set(cpu->ram, address, value);
}

uint8_t cpu_ram_get(CPU *cpu, uint16_t address) {
  return ram_get(cpu->ram, address);
}

void cpu_interrupt(CPU *cpu, uint8_t interrupt) {
  /* NOT IMPLEMENTED */
  printf("Interrupt: %d\n", interrupt);
  exit(1);
}

void cpu_set_flags(CPU *cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c) {
  cpu->f = 0;
  cpu->f |= z << 7;
  cpu->f |= n << 6;
  cpu->f |= h << 5;
  cpu->f |= c << 4;
}

static inline void cpu_push_stack(CPU *cpu, uint16_t value) {
  // decrement the stack pointer
  cpu->sp -= 2;

  // push the value onto the stack
  ram_set(cpu->ram, cpu->sp, value & 0xFF);
  ram_set(cpu->ram, cpu->sp + 1, (value >> 8) & 0xFF);
}

static inline uint8_t get_r8(CPU *cpu, uint8_t opcode) {
  switch(opcode & 0x07) {
    case 0x00: return cpu->b;
    case 0x01: return cpu->c;
    case 0x02: return cpu->d;
    case 0x03: return cpu->e;
    case 0x04: return cpu->h;
    case 0x05: return cpu->l;
    case 0x06: return ram_get(cpu->ram, cpu->hl);
    case 0x07: return cpu->a;
    default: return 0;
  }
}

static inline void set_r8(CPU *cpu, uint8_t opcode, uint8_t value) {
  switch(opcode & 0x07) {
    case 0x00: cpu->b = value; break;
    case 0x01: cpu->c = value; break;
    case 0x02: cpu->d = value; break;
    case 0x03: cpu->e = value; break;
    case 0x04: cpu->h = value; break;
    case 0x05: cpu->l = value; break;
    case 0x06: ram_set(cpu->ram, cpu->hl, value); break;
    case 0x07: cpu->a = value; break;
  }
}

static inline uint16_t get_r16(CPU *cpu, uint8_t opcode) {
  switch(opcode & 0x30) {
    case 0x00: return cpu->bc;
    case 0x10: return cpu->de;
    case 0x20: return cpu->hl;
    case 0x30: return cpu->sp;
    default: return 0;
  }
}

static inline void set_r16(CPU *cpu, uint8_t opcode, uint16_t value) {
  switch(opcode & 0x30) {
    case 0x00: cpu->bc = value; break;
    case 0x10: cpu->de = value; break;
    case 0x20: cpu->hl = value; break;
    case 0x30: cpu->sp = value; break;
  }
}

static inline void compare(CPU *cpu, uint8_t value) {
  uint8_t result = cpu->a - value;

  cpu_set_flags(cpu, result == 0, 1, (cpu->a & 0x0F) < (value & 0x0F), cpu->a < value);
}

static inline void xor_a_r8(CPU *cpu, uint8_t value) {
  cpu->a ^= value;

  cpu_set_flags(cpu, cpu->a == 0, 0, 0, 0);
}

static inline void dec_r8(CPU *cpu, uint8_t opcode) {
  uint8_t value = get_r8(cpu, opcode);
  value--;

  cpu_set_flags(cpu, value == 0, 1, (value & 0x0F) == 0x0F, CARRYF);
  set_r8(cpu, opcode, value);
}

void cpu_init(CPU *cpu, uint8_t *rom) {
  // allocate ram for the cpu
  cpu->ram = malloc(0x10000);

  // copy the rom into the cpu
  memcpy(cpu->ram, rom, 0x8000);

  // set the program counter to entry point
  cpu->pc = 0x100;

  // set the stack pointer to the top of the stack
  cpu->sp = 0xFFFE;

  // set the initial register values
  cpu->a = 0x01;
  cpu->f = 0xB0;
  cpu->b = 0x00;
  cpu->c = 0x13;
  cpu->d = 0x00;
  cpu->e = 0xD8;
  cpu->h = 0x01;
  cpu->l = 0x4D;
}

int cpu_step(CPU *cpu) {
  // fetch the next instruction
  uint8_t opcode = ram_get(cpu->ram, cpu->pc);

  // decode the instruction
  Instruction instruction = instructions[opcode];

  // increment the program counter
  cpu->pc += instruction.bytes;

  // debug current instruction
  printf("0x%04X: 0x%02X, %s\n\n", cpu->pc - instruction.bytes, opcode, instruction.mnemonic);
  // debug registers
  printf("A: 0x%02X, F: 0x%02X, B: 0x%02X, C: 0x%02X, D: 0x%02X, E: 0x%02X, H: 0x%02X, L: 0x%02X\n", cpu->a, cpu->f, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l);
  // debug flags
  printf("Z: %d, N: %d, H: %d, C: %d\n\n", ZEROF, SUBF, HALFF, CARRYF);

  // save operands before
  uint8_t nn = NN;
  uint16_t nnn = NNN;

  switch(opcode) {
    case 0x00: /* NOP */ break;
    CASE4_16(0x01) set_r16(cpu, opcode, nnn); break;
    CASE8_8(0x05) dec_r8(cpu, (opcode - 0x05) / 8); break;
    CASE8_8(0x06) set_r8(cpu, (opcode - 0x06) / 8, nn); break;
    case 0x20: if(!(ZEROF)) cpu->pc += (int8_t) nn; break;
    case 0x32: ram_set(cpu->ram, cpu->hl--, cpu->a); break;
    case 0xA8 ... 0xAF: xor_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xC3: cpu->pc = nnn; break;
    case 0xCD: cpu_push_stack(cpu, cpu->pc); cpu->pc = nnn; break;
    case 0xE0: ram_set(cpu->ram, 0xFF00 + nn, cpu->a); break;
    case 0xF0: cpu->a = ram_get(cpu->ram, 0xFF00 + nn); break;
    case 0xFE: compare(cpu, nn); break;
    case 0xF3: case 0xFB: cpu->ime = opcode == 0xFB; break;
    default: printf("Unknown opcode: 0x%02X, %s at 0x%04X\n", opcode, instruction.mnemonic, cpu->pc - instruction.bytes); exit(1);
  }

  // return the number of cycles
  return instruction.cycles;
}
