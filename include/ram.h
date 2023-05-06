#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

// TODO: move this and use MEM_
#define RAM_DMA  0xFF46
#define RAM_OAM  0xFE00
#define RAM_IO   0xFF00
#define RAM_JOYP 0xFF00

typedef struct Input {
  uint8_t up;
  uint8_t down;
  uint8_t left;
  uint8_t right;
  uint8_t a;
  uint8_t b;
  uint8_t start;
  uint8_t select;
} Input;

// TODO: move input out of here
typedef struct {
  uint8_t data[0x10000];
  Input *input;
} RAM;

void ram_set(RAM *ram, uint16_t address, uint8_t value);
void ram_set_word(RAM *ram, uint16_t address, uint16_t value);
uint8_t ram_get(RAM *ram, uint16_t address);
uint8_t vram_get(RAM *ram, uint16_t address);
uint8_t input_get(Input *input, RAM *ram);

#endif // __MEMORY_H__

