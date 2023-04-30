#include "memory.h"

void memory_set(Memory *memory, uint16_t address, uint8_t value) {
  memory->data[address] = value;
}

uint8_t memory_get(Memory *memory, uint16_t address) {
  return memory->data[address];
}

uint16_t memory_get_word(Memory *memory, uint16_t address) {
  return memory->data[address] | (memory->data[address + 1] << 8);
}
