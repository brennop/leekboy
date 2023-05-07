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
#define CASE_COND_JUMP(x) case x: case x + 8: case x + 16: case x + 24: if(get_flag(cpu, opcode))

#define BIT (1 << ((opcode & 0b00111000) >> 3))

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
  cpu->halted = 0;
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

// TODO: improve this function by not using the flags and instead using the opcode
// as mask to f register
static inline uint8_t get_flag(CPU* cpu, uint8_t opcode) {
  switch(opcode & 0x18) {
    case 0x00: return !(ZEROF);
    case 0x08: return ZEROF;
    case 0x10: return !(CARRYF);
    case 0x18: return CARRYF;
    default: return 0;
  }
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

static inline void sbc_a_r8(CPU *cpu, uint8_t value) {
  uint16_t result = cpu->a - value - (CARRYF);

  uint8_t half_carry = (((cpu->a & 0x0F) - (value & 0x0F) - (CARRYF)) >> 4) & 1;
  uint8_t carry = (result >> 8) & 1;

  cpu->a -= value + (CARRYF);
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

static inline void add_sp(CPU *cpu, uint8_t value, uint16_t *target) {
  int8_t offset = (int8_t)value;
  uint32_t result = cpu->sp + offset;

  // taken from mimic
  // don't know why this works
  int xor = result ^ cpu->sp ^ offset;

  cpu_set_flags(cpu, 0, 0, (xor & 0x10) == 0x10, (xor & 0x100) == 0x100);
  *target = result;
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
  cpu->ram->data[0xFF04] = 0xAB;
  cpu->ram->data[0xFF05] = 0x00;
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
    case 0x20 ... 0x27: // SLA
      set_carry = value >> 7;
      value <<= 1;
      cpu_set_flags(cpu, value == 0, 0, 0, set_carry);
      break;
    case 0x30 ... 0x37: // SWAP
      value = ((value & 0xF) << 4) | ((value & 0xF0) >> 4);
      cpu_set_flags(cpu, value == 0, 0, 0, 0);
      break;
    case 0x38 ... 0x3F:
      set_carry = value & 1;
      value >>= 1;
      cpu_set_flags(cpu, value == 0, 0, 0, set_carry);
      break;
    case 0x40 ... 0x7F: // BIT
      cpu_set_flags(cpu, (value & BIT) == 0, 0, 1, CARRYF);
      break;
    case 0x80 ... 0xBF: // RES
      value &= ~BIT;
      break;
    case 0xC0 ... 0xFF: // SET
      value |= BIT;
      break;
    default: printf("Unknown cb opcode: 0x%02X, %s at 0x%04X\n", opcode, instruction.mnemonic, cpu->pc - instruction.bytes); exit(1);
  }

  set_r8(cpu, opcode, value);
}

inline void trace_01(CPU *cpu) {
  printf("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n", cpu->a, cpu->f, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc, ram_get(cpu->ram, cpu->pc), ram_get(cpu->ram, cpu->pc + 1), ram_get(cpu->ram, cpu->pc + 2), ram_get(cpu->ram, cpu->pc + 3));
}

extern void trace_02(CPU *cpu, Instruction ins) {
  // A:01 F:Z-HC BC:0013 DE:00d8 HL:014d SP:fffe PC:0101 (cy: 12345) | c3 50 01  jp $0150       
  char z_flag = ZEROF ? 'Z' : '-';
  char n_flag = SUBF ? 'N' : '-';
  char h_flag = HALFF ? 'H' : '-';
  char c_flag = CARRYF ? 'C' : '-';

  printf("A:%02x F:%c%c%c%c BC:%04x DE:%04x HL:%04x SP:%04x PC:%04x ", cpu->a, z_flag, n_flag, h_flag, c_flag, cpu->bc, cpu->de, cpu->hl, cpu->sp, cpu->pc);
  printf("(cy: %d) |", cpu->cycles);

  for (int i = 0; i < ins.bytes; i++) {
    printf(" %02x", ram_get(cpu->ram, cpu->pc + i));
  }

  printf(" | %s\n", ins.mnemonic);
}

int cpu_step(CPU *cpu) {
  // check interrupts
  if (cpu->ime) {
    uint8_t interrupt = cpu->ram->data[IE] & cpu->ram->data[IF];
    if (interrupt) {
      // no nested interrupts
      cpu->ime = 0;

      // save current pc to stack
      cpu_push_stack(cpu, cpu->pc);

      if (interrupt & INT_VBLANK) {
        cpu->pc = 0x40;
        cpu->ram->data[IF] &= ~INT_VBLANK;
      } else if (interrupt & INT_LCDSTAT) {
        cpu->pc = 0x48;
        cpu->ram->data[IF] &= ~INT_LCDSTAT;
      } else if (interrupt & INT_TIMER) {
        cpu->pc = 0x50;
        cpu->ram->data[IF] &= ~INT_TIMER;
      } else if (interrupt & INT_SERIAL) {
        cpu->pc = 0x58;
        cpu->ram->data[IF] &= ~INT_SERIAL;
      } else if (interrupt & INT_JOYPAD) {
        cpu->pc = 0x60;
        cpu->ram->data[IF] &= ~INT_JOYPAD;
      } 
    }
  }

  if (cpu->halted) {
    cpu->cycles += 4;
    return 4;
  }

  // fetch the next instruction
  uint8_t opcode = ram_get(cpu->ram, cpu->pc);

  Instruction instruction = instructions[opcode];

  cpu->pc += instruction.bytes;

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
    case 0x07: cpu->f = (cpu->a >> 7) << 4; cpu->a = (cpu->a << 1) | (CARRYF); break;
    case 0x08: ram_set_word(cpu->ram, nnn, cpu->sp); break;
    case 0x12: ram_set(cpu->ram, cpu->de, cpu->a); break;
    case 0x18: cpu->pc += (int8_t) nn; break;
    case 0x1F: cpu->f = (cpu->a & 1) << 4; cpu->a = (cpu->a >> 1) | (carry << 7); break;
    case 0x0A: case 0x1A: cpu->a = ram_get(cpu->ram, get_r16(cpu, opcode)); break;
    case 0x28: if(ZEROF) { cpu->pc += (int8_t) nn; cycles += 4; } break;
    case 0x20: if(!(ZEROF)) { cpu->pc += (int8_t) nn; cycles += 4; } break;
    case 0x30: if(!(CARRYF)) cpu->pc += (int8_t) nn; break;
    case 0x22: ram_set(cpu->ram, cpu->hl++, cpu->a); break;
    case 0x27: daa(cpu); break;
    case 0x2A: cpu->a = ram_get(cpu->ram, cpu->hl++); break;
    case 0x3A: cpu->a = ram_get(cpu->ram, cpu->hl--); break;
    case 0x2F: cpu->a = ~cpu->a; cpu_set_flags(cpu, ZEROF, 1, 1, CARRYF); break;
    case 0x32: ram_set(cpu->ram, cpu->hl--, cpu->a); break;
    case 0x38: if(CARRYF) cpu->pc += (int8_t) nn; break;
    case 0x40 ... 0x75: set_r8(cpu, (opcode - 0x40) / 8, get_r8(cpu, opcode)); break;
    case 0x77 ... 0x7F: set_r8(cpu, (opcode - 0x40) / 8, get_r8(cpu, opcode)); break;
    case 0x76: cpu->halted = 1; break;
    case 0x80 ... 0x87: add_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0x88 ... 0x8F: adc_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0x90 ... 0x97: sub_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0x98 ... 0x9F: sbc_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xA0 ... 0xA7: and_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xA8 ... 0xAF: xor_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xB0 ... 0xB7: or_a_r8(cpu, get_r8(cpu, opcode)); break;
    case 0xB8 ... 0xBF: cp_a_r8(cpu, get_r8(cpu, opcode)); break;
    CASE_COND_JUMP(0xC0) { cpu->pc = cpu_pop_stack(cpu); } break;
    CASE_COND_JUMP(0xC2) { cpu->pc = nnn; } break;
    CASE_COND_JUMP(0xC4) { cpu_push_stack(cpu, cpu->pc); cpu->pc = nnn; cycles += 4; } break;
    case 0xC3: cpu->pc = nnn; break;
    case 0xCE: adc_a_r8(cpu, nn); break;
    case 0xC6: add_a_r8(cpu, nn); break;
    case 0xD6: sub_a_r8(cpu, nn); break;
    case 0xCB: cpu_cb(cpu); break;
    case 0xC1: case 0xD1: case 0xE1: set_r16(cpu, opcode, cpu_pop_stack(cpu)); break;
    case 0xC9: cpu->pc = cpu_pop_stack(cpu); break;
    case 0xCD: cpu_push_stack(cpu, cpu->pc); cpu->pc = nnn; break;
    case 0xDE: sbc_a_r8(cpu, nn); break;
    case 0xD9: cpu->pc = cpu_pop_stack(cpu); cpu->ime = 1; break;
    case 0xE0: ram_set(cpu->ram, 0xFF00 + nn, cpu->a); break;
    case 0xE2: ram_set(cpu->ram, 0xFF00 + cpu->c, cpu->a); break;
    case 0xE6: and_a_r8(cpu, nn); break;
    case 0xEA: ram_set(cpu->ram, nnn, cpu->a); break;
    case 0xE8: add_sp(cpu, nn, &cpu->sp); break;
    case 0xE9: cpu->pc = cpu->hl; break;
    case 0xEE: xor_a_r8(cpu, nn); break;
    case 0xF0: cpu->a = vram_get(cpu->ram, 0xFF00 + nn); break;
    case 0xF1: cpu->af = cpu_pop_stack(cpu) & 0xFFF0; break;
    case 0xF3: case 0xFB: cpu->ime = opcode == 0xFB; break;
    case 0xF6: or_a_r8(cpu, nn); break;
    case 0xF8: add_sp(cpu, nn, &cpu->hl); break;
    case 0xFA: cpu->a = ram_get(cpu->ram, nnn); break;
    case 0xFE: cp_a_r8(cpu, nn); break;
    case 0xC5: case 0xD5: case 0xE5: cpu_push_stack(cpu, get_r16(cpu, opcode)); break;
    case 0xF5: cpu_push_stack(cpu, cpu->af); break;
    case 0xF9: cpu->sp = cpu->hl; break;
    CASE8_8(0xC7) { cpu_push_stack(cpu, cpu->pc); cpu->pc = opcode & 0x38; } break;
    default: printf("Unknown opcode: 0x%02X, %s at 0x%04X\n", opcode, instruction.mnemonic, cpu->pc - instruction.bytes); exit(1);
  }

  cpu->cycles += cycles;
  return cycles;
}
