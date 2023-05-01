#include "ram.h"

void ram_set(RAM *ram, uint16_t address, uint8_t value) {
  ram->data[address] = value;
}

uint8_t ram_get(RAM *ram, uint16_t address) {
  return ram->data[address];
}

// only for testing
uint8_t vram_get(RAM *ram, uint16_t address) {
  if (address == 0xFF44) return 0x90;
  return ram_get(ram, address);
}

