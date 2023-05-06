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

/**
 * LCDC
 * Bit 7 - LCD Display Enable             (0=Off, 1=On)
 * Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
 * Bit 5 - Window Display Enable          (0=Off, 1=On)
 * Bit 4 - BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF)
 * Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF)
 * Bit 2 - OBJ (Sprite) Size              (0=8x8, 1=8x16)
 * Bit 1 - OBJ (Sprite) Display Enable    (0=Off, 1=On)
 * Bit 0 - BG Display                     (0=Off, 1=On)
 */
#define LCDC_LCD_ENABLE       0x80
#define LCDC_WND_TILEMAP_AREA 0x40
#define LCDC_WND_ENABLE       0x20
#define LCDC_TILE_DATA_AREA   0x10
#define LCDC_BG_TILEMAP_AREA  0x08
#define LCDC_OBJ_SIZE         0x04
#define LCDC_OBJ_ENABLE       0x02
#define LCDC_BG_ENABLE        0x01

// palette mem
#define PAL_BGP 0xFF47

typedef enum {
  MODE_HBLANK = 0,
  MODE_VBLANK = 1,
  MODE_OAM    = 2,
  MODE_VRAM   = 3
} Mode;

typedef struct {
  RAM *ram;
  CPU *cpu;
  Mode mode;

  int framebuffer[160 * 144];
  int cycles;
  int scanline;
} GPU;

void gpu_init(GPU *gpu, CPU *cpu, RAM *ram);
void gpu_step(GPU *gpu, int cycles);
void gpu_render_scanline(GPU *gpu);

#endif // __GPU_H__
