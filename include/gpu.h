#ifndef __GPU_H__
#define __GPU_H__

#include "ram.h"
#include "cpu.h"

#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY  0xFF42
#define SCX  0xFF43
#define LY   0xFF44
#define LYC  0xFF45
#define WY   0xFF4A
#define WX   0xFF4B

typedef struct {
  RAM *ram;
  CPU *cpu;

  uint8_t framebuffer[160][144];
  int scanline;
} GPU;

void gpu_init(GPU *gpu, CPU *cpu, RAM *ram);
void gpu_step(GPU *gpu, int cycles);
void gpu_render_scanline(GPU *gpu);

#endif // __GPU_H__
