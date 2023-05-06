#include "ram.h"
#include <stdio.h>

void ram_set(RAM *ram, uint16_t address, uint8_t value) {
  if (address == RAM_DMA) {
    uint16_t src = value << 8;
    for (int i = 0; i < 0xA0; i++) {
      ram->data[RAM_OAM + i] = ram->data[src + i];
    }
    return;
  }

  ram->data[address] = value;
}

void ram_set_word(RAM *ram, uint16_t address, uint16_t value) {
  ram_set(ram, address, value & 0xFF);
  ram_set(ram, address + 1, value >> 8);
}

uint8_t ram_get(RAM *ram, uint16_t address) {
  return ram->data[address];
}

// only for testing
uint8_t vram_get(RAM *ram, uint16_t address) {
#ifdef ROM_TEST
  if (address == 0xFF44) return 0x90;
#endif
  return ram_get(ram, address);
}

