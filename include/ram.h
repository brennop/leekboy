#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

// TODO: move this and use MEM_
#define RAM_DMA  0xFF46
#define RAM_OAM  0xFE00
#define RAM_IO   0xFF00
#define RAM_JOYP 0xFF00
#define RAM_VRAM 0x8000

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

typedef enum Mapper {
  MAP_NONE,
  MAP_MBC1,
} Mapper;

typedef enum {
  BANK_ROM,
  BANK_RAM,
} BankMode;

#define RAM_BANK_SIZE 0x2000

// TODO: move input out of here
typedef struct {
  Input *input;

  Mapper mapper;
  uint8_t rom_bank;
  uint8_t ram_bank;
  uint8_t ram_enable;
  BankMode bank_mode;

  uint8_t data[0x10000];
  uint8_t banks[0x8000];
  uint8_t *rom;
} RAM;

void ram_init(RAM *ram, Input *input, uint8_t *rom);
void ram_set(RAM *ram, uint16_t address, uint8_t value);
void ram_set_word(RAM *ram, uint16_t address, uint16_t value);
uint8_t ram_get(RAM *ram, uint16_t address);

#endif // __MEMORY_H__

