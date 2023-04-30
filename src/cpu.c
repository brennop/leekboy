#include "cpu.h"
#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cpu_init(CPU *cpu, uint8_t *rom) {
  // allocate memory for the cpu
  cpu->memory = malloc(0x10000);

  // copy the rom into the cpu
  memcpy(cpu->memory, rom, 0x8000);

  // set the program counter to entry point
  cpu->pc = 0x100;

  // set the stack pointer to the top of the stack
  cpu->sp = 0xFFFE;
}

int cpu_step(CPU *cpu) {
  // fetch the next instruction
  uint8_t i = cpu->memory[cpu->pc];

  // decode the instruction
  Instruction instruction = instructions[i];

  if (instruction.execute == NULL) {
    printf("Unimplemented instruction: 0x%02X, %s\n", i, instruction.mnemonic);
    exit(1);
  }

  // increment the program counter
  cpu->pc += instruction.bytes;

  // execute the instruction
  instruction.execute(cpu);

  // return the number of cycles the instruction took
  return instruction.cycles;
}
