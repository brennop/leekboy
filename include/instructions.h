#ifndef __INSTRUCTIONS_H__
#define __INSTRUCTIONS_H__

#include "cpu.h"
#include <stdint.h>

typedef struct {
  char *mnemonic;
  uint8_t bytes;
  uint8_t cycles;
} Instruction;

extern Instruction instructions[256];
extern Instruction prefixed[256];

#endif // __INSTRUCTIONS_H__
