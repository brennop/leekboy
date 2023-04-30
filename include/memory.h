#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

typedef struct {
  uint8_t data[0x10000];
} Memory;

void memory_set(Memory *memory, uint16_t address, uint8_t value);
uint8_t memory_get(Memory *memory, uint16_t address);

#endif // __MEMORY_H__

