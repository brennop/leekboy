#include <stdio.h>

#include "ram.h"

#define MBC1 ram->mapper == MAP_MBC1
#define MBC3 ram->mapper == MAP_MBC3

uint8_t input_get(Input *input, RAM *ram) {
  /** raw get to prevent infinite recursion */
  uint8_t joypad = ~ram->data[RAM_JOYP] & 0x30;

  if ((joypad & 0x10)) {
    if (input->right)
      joypad |= 0x01;
    if (input->left)
      joypad |= 0x02;
    if (input->up)
      joypad |= 0x04;
    if (input->down)
      joypad |= 0x08;
  }
  if ((joypad & 0x20)) {
    if (input->a)
      joypad |= 0x01;
    if (input->b)
      joypad |= 0x02;
    if (input->select)
      joypad |= 0x04;
    if (input->start)
      joypad |= 0x08;
  }

  return (~joypad & 0x3F) | 0xC0;
}

void ram_init(RAM *ram, Input *input, uint8_t *rom) {
  ram->input = input;
  ram->rom = rom;

  ram->rom_bank = 1;
  ram->ram_bank = 0;
  ram->ram_enable = 0;
  ram->bank_mode = BANK_ROM;

  uint8_t cart_info = ram->rom[0x147];
  switch (cart_info) {
  case 0x00:
    ram->mapper = MAP_NONE;
    break;
  case 0x01 ... 0x03:
    ram->mapper = MAP_MBC1;
    break;
  }
}

void ram_set(RAM *ram, uint16_t address, uint8_t value) {
  switch (address >> 12) {
  case 0x0 ... 0x1:
    if (MBC1)
      ram->ram_enable = (value & 0x0A) == 0x0A;
    break;
  case 0x2 ... 0x3:
    if (MBC1)
      ram->rom_bank = (ram->rom_bank & 0x60) | (value & 0x1F);
    break;
  case 0x4 ... 0x5:
    if (MBC1) {
      value &= 0x03;
      switch (ram->bank_mode) {
      case BANK_RAM:
        ram->ram_bank = value;
        break;
      case BANK_ROM:
        ram->rom_bank = (ram->rom_bank & 0x1F) | (value << 5);
        break;
      }
    }
    break;
  case 0x6 ... 0x7:
    if (MBC1) {
      if (value & 0x01) {
        ram->bank_mode = BANK_RAM;
      } else {
        ram->bank_mode = BANK_ROM;
        ram->ram_bank = 0;
      }
    }
    break;
  case 0xA ... 0xB:
    switch (ram->mapper) {
    case MAP_NONE:
      break;
    case MAP_MBC1:
      if (ram->ram_enable)
        ram->banks[address - 0xA000 + ram->ram_bank * 0x2000] = value;
      break;
    }
  case 0xC ... 0xD:
    // work ram
    break;
  case 0xE ... 0xF:
    if (address <= 0xFDFF) {
      ram->data[address - 0x2000] = value;
    } else if (address == RAM_DMA) {
      uint16_t src = value << 8;
      for (int i = 0; i < 0xA0; i++) {
        ram->data[RAM_OAM + i] = ram->data[src + i];
      }
    }
    break;
  }

  ram->data[address] = value;
}

uint8_t ram_get(RAM *ram, uint16_t address) {
  switch (address >> 12) {
  case 0x0 ... 0x3:
    return ram->rom[address];
    break;
  case 0x4 ... 0x7:
    return ram->rom[address - 0x4000 + ram->rom_bank * 0x4000];
    break;
  case 0x8 ... 0x9:
    break;
  case 0xA ... 0xB:
    switch (ram->mapper) {
    case MAP_NONE:
      break;
    case MAP_MBC1:
      if (ram->ram_enable)
        return ram->banks[address - 0xA000 + ram->ram_bank * 0x2000];
      break;
    }
  case 0xC ... 0xD:
    // work ram
    break;
  case 0xE ... 0xF:
    if (address <= 0xFDFF) {
      return ram->data[address - 0x2000];
    } else if (address == RAM_JOYP) {
      return input_get(ram->input, ram);
    }
    break;
  }

  return ram->data[address];
}

// TODO: remove this
void ram_set_word(RAM *ram, uint16_t address, uint16_t value) {
  ram_set(ram, address, value & 0xFF);
  ram_set(ram, address + 1, value >> 8);
}
