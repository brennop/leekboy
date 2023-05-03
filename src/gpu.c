#include "gpu.h"
#include "cpu.h"
#include "ram.h"
#include <stdio.h>

void gpu_init(GPU *gpu, CPU *cpu, RAM *ram) {
  gpu->cpu = cpu;
  gpu->ram = ram;
  gpu->scanline = 456;
}

static inline uint8_t gpu_is_lcd_enabled(GPU *gpu) {
  return ram_get(gpu->ram, LCDC) & 0x80;
}

void gpu_set_status(GPU *gpu) {
  uint8_t status = ram_get(gpu->ram, STAT);

  if (!gpu_is_lcd_enabled(gpu)) {
    gpu->scanline = 456;
    ram_set(gpu->ram, LY, 0);
    status = (status & 0xFC) | 0x01;
    ram_set(gpu->ram, STAT, status);
    return;
  }

  int current_line = vram_get(gpu->ram, LY);
  int current_mode = status & 0x03;
  uint8_t mode = 0;
  uint8_t req_int = 0;

  if (current_line >= 144) { /* V-Blank */
    mode = 1;
    status |= 0x01;
    status &= ~0x02;
    req_int = status & 0x10;
  } else {
    if (gpu->scanline >= 376) {
      mode = 2;
      status |= 0x02;
      status &= ~0x01;
      req_int = status & 0x20;
    } else if (gpu->scanline >= 204) {
      mode = 3;
      status |= 0x03;
      /* req_int = status & 0x30; */ // FIXME: check why this was suggested
    } else {
      mode = 0;
      status &= ~0x03;
      req_int = status & 0x08;
    }

    // mode changed, request interrupt
    if (req_int && (mode != current_mode)) {
      cpu_interrupt(gpu->cpu, INT_LCDSTAT);
    }

    // check coincidence
    if (vram_get(gpu->ram, LY) == ram_get(gpu->ram, LYC)) {
      status |= 0x04;
      if (status & 0x40) {
        cpu_interrupt(gpu->cpu, INT_LCDSTAT);
      }
    } else {
      status &= ~0x04;
    }

    ram_set(gpu->ram, STAT, status);
  }
}

void gpu_step(GPU *gpu, int cycles) {
  gpu_set_status(gpu);

  if (!gpu_is_lcd_enabled(gpu)) {
    return;
  }

  gpu->scanline -= cycles;

  if (gpu->scanline <= 0) {
    // move to next scanline
    gpu->ram->data[LY]++;

    int current_line = vram_get(gpu->ram, LY);

    gpu->scanline += 456;

    if (current_line == 144) {
      // request vblank interrupt
      /* cpu_interrupt(gpu->cpu, INT_VBLANK); */
    } else if (current_line > 153) {
      gpu->ram->data[LY] = 0;
    } else if (current_line < 144) {
      gpu_render_scanline(gpu);
    }
  }
}

void gpu_render_scanline(GPU *gpu) {
}

