#include <stdio.h>

#include "ram.h"

uint8_t input_get(Input *input, RAM *ram) {
  // this check is only for testing
  // TODO: remove this check
  if (input == NULL) return 0xFF;

  /** raw get to prevent infinite recursion */
  uint8_t joypad = ~ram->data[RAM_JOYP];

  if ((joypad & 0x10) == 0) {
    if (input->up) joypad &= ~0x08;
    if (input->down) joypad &= ~0x04;
    if (input->left) joypad &= ~0x02;
    if (input->right) joypad &= ~0x01;
  }
  if ((joypad & 0x20) == 0) {
    if (input->a) joypad &= ~0x08;
    if (input->b) joypad &= ~0x04;
    if (input->select) joypad &= ~0x02;
    if (input->start) joypad &= ~0x01;
  }

  return joypad;
}

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
  if (address == RAM_JOYP) {
    return input_get(ram->input, ram);
  }

  return ram->data[address];
}

// only for testing
uint8_t vram_get(RAM *ram, uint16_t address) {
#ifdef ROM_TEST
  if (address == 0xFF44) return 0x90;
#endif
  return ram_get(ram, address);
}

