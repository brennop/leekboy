#include "gpu.h"
#include "cpu.h"
#include "ram.h"

#include <stdbool.h>
#include <stdio.h>

const int colors[] = {0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000};

static void gpu_render_tiles(GPU *gpu);
static void gpu_render_sprites(GPU *gpu);
static inline int get_color(GPU *gpu, uint8_t value, uint16_t pallete);

void gpu_init(GPU *gpu, CPU *cpu, RAM *ram) {
  gpu->cpu = cpu;
  gpu->ram = ram;

  for (int i = 0; i < 160 * 144; i++) {
    gpu->framebuffer[i] = 0x00;
  }
}

static inline uint8_t gpu_is_lcd_enabled(GPU *gpu) {
  return ram_get(gpu->ram, LCDC) & 0x80;
}

void gpu_update_stat(GPU *gpu, Mode mode) {
  uint8_t stat = ram_get(gpu->ram, STAT) & 0xFC;
  uint8_t ly = ram_get(gpu->ram, LY);
  uint8_t lyc = ram_get(gpu->ram, LYC);

  if (ly == lyc) {
    stat |= STAT_LYC_LY;
    if (stat & STAT_LYC_INT) {
      cpu_interrupt(gpu->cpu, INT_LCDSTAT);
    }
  } else {
    stat &= ~STAT_LYC_LY;
  }

  ram_set(gpu->ram, STAT, stat | mode);
}

void gpu_set_mode(GPU *gpu, Mode mode) {
  gpu_update_stat(gpu, mode);
  uint8_t stat = ram_get(gpu->ram, STAT);

  switch (mode) {
  case MODE_OAM:
    if (stat & STAT_OAM_INT)
      cpu_interrupt(gpu->cpu, INT_LCDSTAT);
    break;
  case MODE_VRAM:
    break;
  case MODE_HBLANK:
    if (stat & STAT_HBL_INT)
      cpu_interrupt(gpu->cpu, INT_LCDSTAT);
    break;
  case MODE_VBLANK:
    cpu_interrupt(gpu->cpu, INT_VBLANK);
    if (stat & STAT_VBL_INT)
      cpu_interrupt(gpu->cpu, INT_LCDSTAT);
    break;
  }

  gpu->mode = mode;
}

void gpu_step(GPU *gpu, int cycles) {
  if (!gpu_is_lcd_enabled(gpu))
    return;

  gpu->cycles += cycles;

  uint8_t ly = ram_get(gpu->ram, LY);

  switch (gpu->mode) {
  case MODE_OAM:
    if (gpu->cycles >= 80) {
      gpu->cycles -= 80;
      gpu_set_mode(gpu, MODE_VRAM);
    }
    break;
  case MODE_VRAM:
    if (gpu->cycles >= 172) {
      gpu->cycles -= 172;
      gpu_set_mode(gpu, MODE_HBLANK);
      gpu_render_scanline(gpu);
    }
    break;
  case MODE_HBLANK:
    if (gpu->cycles >= 204) {
      gpu->cycles -= 204;
      ram_set(gpu->ram, LY, ly + 1);

      if (ly == 143) {
        gpu_set_mode(gpu, MODE_VBLANK);
      } else {
        gpu_set_mode(gpu, MODE_OAM);
      }
    }
    break;
  case MODE_VBLANK:
    if (gpu->cycles >= 456) {
      gpu->cycles -= 456;
      ram_set(gpu->ram, LY, ly + 1);

      if (ly == 153) {
        gpu_set_mode(gpu, MODE_OAM);
        ram_set(gpu->ram, LY, 0);
      }
    }
    break;
  }
}

void gpu_render_scanline(GPU *gpu) {
  uint8_t lcdc = ram_get(gpu->ram, LCDC);

  if (lcdc & LCDC_BG_ENABLE) {
    gpu_render_tiles(gpu);
  }

  if (lcdc & LCDC_OBJ_ENABLE) {
    gpu_render_sprites(gpu);
  }
}

static void gpu_render_tiles(GPU *gpu) {
  uint8_t lcdc = ram_get(gpu->ram, LCDC);

  uint8_t scroll_y = ram_get(gpu->ram, SCY);
  uint8_t scroll_x = ram_get(gpu->ram, SCX);
  uint8_t window_y = ram_get(gpu->ram, WY);
  uint8_t window_x = ram_get(gpu->ram, WX) - 7;

  uint8_t window_enabled = lcdc & LCDC_WND_ENABLE;

  int ly = ram_get(gpu->ram, LY);

  // is current scanline within window?
  uint8_t using_window = window_enabled && (window_y <= ly);

  // which tile data are we using?
  uint8_t is_unsigned = lcdc & LCDC_TILE_DATA_AREA;
  uint16_t tile_data = is_unsigned ? 0x8000 : 0x8800;

  // which bg memory are we using?
  uint16_t background_memory =
      lcdc & (using_window ? LCDC_WND_TILEMAP_AREA : LCDC_BG_TILEMAP_AREA)
          ? 0x9C00
          : 0x9800;

  // which of the 32 vertical tiles does the scanline fall into?
  uint8_t y_pos = using_window ? ly - window_y : scroll_y + ly;

  // which of the 8 vertical pixels of the tile does the scanline fall into?
  uint16_t tile_row = (y_pos >> 3) << 5;

  // draw each pixel of this scanline
  for (int pixel = 0; pixel < 160; pixel++) {
    uint8_t x_pos = pixel + scroll_x;

    // if window, translate to window space
    if (using_window && pixel >= window_x) {
      x_pos = pixel - window_x;
    }

    // which of the 32 horizontal tiles does this pixel fall into?
    uint16_t tile_col = x_pos >> 3;

    uint16_t tile_address = background_memory + tile_row + tile_col;
    int16_t tile_num = is_unsigned ? (uint8_t)ram_get(gpu->ram, tile_address)
                                   : (int8_t)ram_get(gpu->ram, tile_address);

    // if not unsigned, +128 to tile number
    uint16_t tile_location =
        tile_data + (is_unsigned ? tile_num : tile_num + 128) * 16;

    uint8_t line = (y_pos % 8) << 1;

    uint8_t data_right = ram_get(gpu->ram, tile_location + line);
    uint8_t data_left = ram_get(gpu->ram, tile_location + line + 1);

    // TODO: understand this
    int color_bit = ((x_pos % 8) - 7) * -1;

    // combine data
    int color_num = ((data_left & (1 << color_bit)) >> color_bit) << 1 |
                    (data_right & (1 << color_bit)) >> color_bit;

    // write color to framebuffer
    gpu->framebuffer[pixel + (ly * 160)] =
        get_color(gpu, color_num, ram_get(gpu->ram, PAL_BGP));
  }
}

static void gpu_render_sprites(GPU *gpu) {
  uint8_t lcdc = ram_get(gpu->ram, LCDC);
  uint8_t ly = ram_get(gpu->ram, LY);

  bool big_sprites = lcdc & LCDC_OBJ_SIZE;

  for (int sprite = 0; sprite < 40; sprite++) {
    int index = sprite << 2;
    uint8_t y_pos = ram_get(gpu->ram, RAM_OAM + index) - 16;
    uint8_t x_pos = ram_get(gpu->ram, RAM_OAM + index + 1) - 8;

    uint8_t tile_location = ram_get(gpu->ram, RAM_OAM + index + 2);
    uint8_t attributes = ram_get(gpu->ram, RAM_OAM + index + 3);

    bool y_flip = attributes & OBJ_Y_FLIP;
    bool x_flip = attributes & OBJ_X_FLIP;
    int height = big_sprites ? 16 : 8;

    // should we draw this sprite?
    if (ly >= y_pos && ly < (y_pos + height)) {
      int line = ly - y_pos;

      if (y_flip) {
        line -= height;
        line *= -1;
      }

      line <<= 1; // same formula as gpu_render_tiles
      uint16_t address = RAM_VRAM + (tile_location << 4) + line;
      uint8_t data_right = ram_get(gpu->ram, address);
      uint8_t data_left = ram_get(gpu->ram, address + 1);

      for (int tile_pixel = 7; tile_pixel >= 0; tile_pixel--) {
        int color_bit = tile_pixel;

        if (x_flip) {
          color_bit -= 7;
          color_bit *= -1;
        }

        int color_num = ((data_left & (1 << color_bit)) >> color_bit) << 1 |
                        (data_right & (1 << color_bit)) >> color_bit;

        uint16_t palette = attributes & OBJ_PALETTE_NUMBER ? 0xFF49 : 0xFF48;
        int color = get_color(gpu, color_num, ram_get(gpu->ram, palette));

        // white is transparent for sprites
        if (color == 0xFFFFFF) {
          continue;
        }

        int x = x_pos + (7 - tile_pixel);

        if (x < 0 || x >= 160) {
          continue;
        }

        gpu->framebuffer[x + (ly * 160)] = color;
      }
    }
  }
}

static inline int get_color(GPU *gpu, uint8_t value, uint16_t palette) {
  return colors[((palette >> ((value << 1) | 1)) & 0x01) << 1 |
                ((palette >> (value << 1)) & 0x01)];
}
