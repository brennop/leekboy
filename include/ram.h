#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

typedef struct {
  uint8_t data[0x10000];
} RAM;

void ram_set(RAM *ram, uint16_t address, uint8_t value);
void ram_set_word(RAM *ram, uint16_t address, uint16_t value);
uint8_t ram_get(RAM *ram, uint16_t address);
uint8_t vram_get(RAM *ram, uint16_t address);

#endif // __MEMORY_H__

