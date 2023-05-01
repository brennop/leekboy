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

void cpu_memory_set(CPU *cpu, uint16_t address, uint8_t value) {
  ram_set(cpu->ram, address, value);
}

uint8_t cpu_memory_get(CPU *cpu, uint16_t address) {
  return ram_get(cpu->ram, address);
}

void cpu_interrupt(CPU *cpu, uint8_t interrupt) {
  uint8_t interrupt_flag = ram_get(cpu->ram, IF);
  interrupt_flag |= interrupt;
  ram_set(cpu->ram, IF, interrupt_flag);
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

static inline uint16_t cpu_pop_stack(CPU *cpu) {
  // pop the value off the stack
  uint16_t value = ram_get(cpu->ram, cpu->sp) | (ram_get(cpu->ram, cpu->sp + 1) << 8);
  // increment the stack pointer
  cpu->sp += 2;

  return value;
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

static inline void daa(CPU *cpu) {
  uint8_t correction = 0;
  uint8_t set_carry = 0;

  if (HALFF || (!(SUBF) && (cpu->a & 0x0F) > 9)) {
    correction |= 0x06;
  }

  if (CARRYF || (!(SUBF) && cpu->a > 0x99)) {
    correction |= 0x60;
    set_carry = 1;
  }

  if (SUBF) {
    cpu->a -= correction;
  } else {
    cpu->a += correction;
  }

  cpu_set_flags(cpu, cpu->a == 0, SUBF, 0, set_carry);
}

static inline void cp_a_r8(CPU *cpu, uint8_t value) {
  uint8_t result = cpu->a - value;

  cpu_set_flags(cpu, result == 0, 1, (cpu->a & 0x0F) < (value & 0x0F), cpu->a < value);
}

static inline void add_a_r8(CPU *cpu, uint8_t value) {
  cpu->a += value;
  cpu_set_flags(cpu, cpu->a == 0, 0, (cpu->a & 0x0F) < (value & 0x0F), cpu->a < value);
}

static inline void adc_a_r8(CPU *cpu, uint8_t value) {
  uint16_t result = cpu->a + value + (CARRYF);

  uint8_t half_carry = (((cpu->a & 0x0F) + (value & 0x0F) + (CARRYF)) >> 4) & 1;
  uint8_t carry = (result >> 8) & 1;

  cpu->a += value + (CARRYF);
  cpu_set_flags(cpu, cpu->a == 0, 0, half_carry, carry);
}

static inline void sub_a_r8(CPU *cpu, uint8_t value) {
  uint16_t result = cpu->a - value;

  uint8_t half_carry = (((cpu->a & 0x0F) - (value & 0x0F)) >> 4) & 1;
  uint8_t carry = (result >> 8) & 1;

  cpu->a -= value;
  cpu_set_flags(cpu, cpu->a == 0, 1, half_carry, carry);
}

static inline void xor_a_r8(CPU *cpu, uint8_t value) {
  cpu->a ^= value;
  cpu_set_flags(cpu, cpu->a == 0, 0, 0, 0);
}

static inline void or_a_r8(CPU *cpu, uint8_t value) {
  cpu->a |= value;
  cpu_set_flags(cpu, cpu->a == 0, 0, 0, 0);
}

static inline void and_a_r8(CPU *cpu, uint8_t value) {
  cpu->a &= value;
  cpu_set_flags(cpu, cpu->a == 0, 0, 1, 0);
}

static inline void inc_r8(CPU *cpu, uint8_t opcode) {
  uint8_t value = get_r8(cpu, opcode);
  value++;

  cpu_set_flags(cpu, value == 0, 0, (value & 0x0F) == 0, CARRYF);
  set_r8(cpu, opcode, value);
}

static inline void dec_r8(CPU *cpu, uint8_t opcode) {
  uint8_t value = get_r8(cpu, opcode);
  value--;

  cpu_set_flags(cpu, value == 0, 1, (value & 0x0F) == 0x0F, CARRYF);
  set_r8(cpu, opcode, value);
}

static inline void add_hl_r16(CPU *cpu, uint8_t opcode) {
  uint16_t value = get_r16(cpu, opcode);
  uint32_t result = cpu->hl + value;

  cpu_set_flags(cpu, ZEROF, 0, (result & 0x0FFF) < (cpu->hl & 0x0FFF), (result & 0xFFFF) < (cpu->hl & 0xFFFF));
  cpu->hl += value;
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

  // set memory values
  // TODO: check missing values
  // https://gbdev.io/pandocs/Power_Up_Sequence.html#hardware-registers
  cpu->ram->data[0xFF00] = 0xCF;
  cpu->ram->data[0xFF40] = 0x91;
  cpu->ram->data[0xFF41] = 0x85;
  cpu->ram->data[0xFF42] = 0x00;
  cpu->ram->data[0xFF43] = 0x00;
  cpu->ram->data[0xFF45] = 0x00;
  cpu->ram->data[0xFF46] = 0xFF;
  cpu->ram->data[0xFF47] = 0xFC;
}

static void cpu_cb(CPU *cpu) {
  uint8_t opcode = ram_get(cpu->ram, cpu->pc);
  Instruction instruction = prefixed[opcode];

  // remove 1 that was added on main step
  cpu->pc += instruction.bytes - 1;

  uint8_t value = get_r8(cpu, opcode);
  uint8_t carry = CARRYF;
  uint8_t set_carry = 0;

  switch(opcode) {
    case 0x18 ... 0x1F: // RR
      set_carry = value & 1;
      value >>= 1;
      if(carry) value |= 0x80;
      cpu_set_flags(cpu, value == 0, 0, 0, set_carry);
      break;
    case 0x38 ... 0x3F:
      set_carry = value & 1;
      value >>= 1;
      cpu_set_flags(cpu, value == 0, 0, 0, set_carry);
      break;
    default: printf("Unknown cb opcode: 0x%02X, %s at 0x%04X\n", opcode, instruction.mnemonic, cpu->pc - instruction.bytes); exit(1);
  }

  set_r8(cpu, opcode, value);
}

int cpu_step(CPU *cpu) {
  // log
  // eg. A:00 F:11 B:22 C:33 D:44 E:55 H:66 L:77 SP:8888 PC:9999 PCMEM:AA,BB,CC,DD
  printf("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n", cpu->a, cpu->f, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc, ram_get(cpu->ram, cpu->pc), ram_get(cpu->ram, cpu->pc + 1), ram_get(cpu->ram, cpu->pc + 2), ram_get(cpu->ram, cpu->pc + 3));
  // track 0xFF44 (LY), 0xFF40 (LCDC), 0xFF41 (STAT)
  printf("LY: %02X, LCDC: %02X, STAT: %02X\n", ram_get(cpu->ram, 0xFF44), ram_get(cpu->ram, 0xFF40), ram_get(cpu->ram, 0xFF41));

  // fetch the next instruction
  uint8_t opcode = ram_get(cpu->ram, cpu->pc);

  // decode the instruction
  Instruction instruction = instructions[opcode];

  if (cpu->pc == 0x2817) {
    getchar();
  }

  // increment the program counter
  cpu->pc += instruction.bytes;

  // debug current instruction
  /* printf("0x%04X: 0x%02X, %s\n\n", cpu->pc - instruction.bytes, opcode, instruction.mnemonic); */
  /* // debug registers */
  /* printf("A: 0x%02X, F: 0x%02X, B: 0x%02X, C: 0x%02X, D: 0x%02X, E: 0x%02X, H: 0x%02X, L: 0x%02X\n", cpu->a, cpu->f, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l); */
  /* // debug flags */
  /* printf("Z: %d, N: %d, H: %d, C: %d\n\n", ZEROF, SUBF, HALFF, CARRYF); */

  // save operands before
  uint8_t nn = NN;
  uint16_t nnn = NNN;

  uint8_t carry = CARRYF;
  uint8_t cycles = instruction.cycles;

  switch(opcode) {
    case 0x00: /* NOP */ break;
    CASE4_16(0x01) set_r16(cpu, opcode, nnn); break;
    CASE8_8(0x04) inc_r8(cpu, (opcode - 0x04) / 8); break;
    CASE8_8(0x05) dec_r8(cpu, (opcode - 0x05) / 8); break;
    CASE8_8(0x06) set_r8(cpu, (opcode - 0x06) / 8, nn); break;
    CASE4_16(0x09) add_hl_r16(cpu, opcode); break;
    CASE4_16(0x03) set_r16(cpu, opcode, get_r16(cpu, opcode) + 1); break;
    CASE4_16(0x0B) set_r16(cpu, opcode, get_r16(cpu, opcode) - 1); break;
    case 0x12: ram_set(cpu->ram, cpu->de, cpu->a); break;
    case 0x18: cpu->pc += (int8_t) nn; break;
    case 0x1F: cpu->f = (cpu->a & 1) << 4; cpu->a = (cpu->a >> 1) | (carry << 7); break;
    case 0x0A: case 0x1A: cpu->a = ram_get(cpu->ram, get_r16(cpu, opcode)); break;
    case 0x28: if(ZEROF) cpu->pc += (int8_t) nn; break;
    case 0x20: if(!(ZEROF)) { cpu->pc += (int8_t) nn; cycles += 4; } break;
    case 0x30: if(!(CARRYF)) cpu->pc += (int8_t) nn; break;
    case 0x22: ram_set(cpu->ram, cpu->hl++, cpu->a); break;
    case 0x27: daa(cpu); break;
    case 0x2A: cpu->a = ram_get(cpu->ram, cpu->hl++); break;
    case 0x2F: cpu->a = ~cpu->a; cpu_set_flags(cpu, ZEROF, 1, 1, CARRYF); break;
    case 0x32: ram_set(cpu->ram, cpu->hl--, cpu->a); break;
    case 0x38: if(CARRYF) cpu->pc += (int8_t) nn; break;
    case 0x40 ... 0x7F: /* FIXME: HALT */ set_r8(cpu, (opcode - 0x40) / 8, get_r8(cpu, opcode)); break;
    case 0x80 ... 0x87: add_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0x88 ... 0x8F: adc_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0x90 ... 0x97: sub_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xA0 ... 0xA7: and_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xA8 ... 0xAF: xor_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xB0 ... 0xB7: or_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xB8 ... 0xBF: cp_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xC2: if(!(ZEROF)) cpu->pc = nnn; break;
    case 0xC3: cpu->pc = nnn; break;
    case 0xC4: if(!(ZEROF)) { cpu_push_stack(cpu, cpu->pc); cpu->pc = nnn; } break;
    case 0xC8: if(ZEROF) cpu->pc = cpu_pop_stack(cpu); break;
    case 0xCE: adc_a_r8(cpu, nn); break;
    case 0xC6: add_a_r8(cpu, nn); break;
    case 0xD6: sub_a_r8(cpu, nn); break;
    case 0xD8: if(CARRYF) cpu->pc = cpu_pop_stack(cpu); break;
    case 0xCB: cpu_cb(cpu); break;
    case 0xC1: case 0xD1: case 0xE1: set_r16(cpu, opcode, cpu_pop_stack(cpu)); break;
    case 0xF1: cpu->af = cpu_pop_stack(cpu) & 0xFFF0; break;
    case 0xC9: cpu->pc = cpu_pop_stack(cpu); break;
    case 0xCD: cpu_push_stack(cpu, cpu->pc); cpu->pc = nnn; break;
    case 0xD0: if(!(CARRYF)) cpu->pc = cpu_pop_stack(cpu); break;
    case 0xE0: ram_set(cpu->ram, 0xFF00 + nn, cpu->a); break;
    case 0xE2: ram_set(cpu->ram, 0xFF00 + cpu->c, cpu->a); break;
    case 0xE6: and_a_r8(cpu, nn); break;
    case 0xEA: ram_set(cpu->ram, nnn, cpu->a); break;
    case 0xE9: cpu->pc = cpu->hl; break;
    case 0xEE: xor_a_r8(cpu, nn); break;
    case 0xF0: cpu->a = vram_get(cpu->ram, 0xFF00 + nn); break;
    case 0xF3: case 0xFB: cpu->ime = opcode == 0xFB; break;
    case 0xFA: cpu->a = ram_get(cpu->ram, nnn); break;
    case 0xFE: cp_a_r8(cpu, nn); break;
    case 0xC5: case 0xD5: case 0xE5: cpu_push_stack(cpu, get_r16(cpu, opcode)); break;
    case 0xF5: cpu_push_stack(cpu, cpu->af); break;
    default: printf("Unknown opcode: 0x%02X, %s at 0x%04X\n", opcode, instruction.mnemonic, cpu->pc - instruction.bytes); exit(1);
  }

  // return the number of cycles
  return cycles;
}
