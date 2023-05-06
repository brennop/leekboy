#include "gpu.h"
#include "cpu.h"
#include "ram.h"
#include <stdio.h>

const int colors[] = {0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000};

static void gpu_render_tiles(GPU *gpu);
static void gpu_render_sprites(GPU *gpu);
static inline int get_color(GPU *gpu, uint8_t value, uint16_t pallete);

void gpu_init(GPU *gpu, CPU *cpu, RAM *ram) {
  gpu->cpu = cpu;
  gpu->ram = ram;
  gpu->scanline = 456;

  for (int i = 0; i < 160 * 144; i++) {
    gpu->framebuffer[i] = 0x00;
  }
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
      cpu_interrupt(gpu->cpu, INT_VBLANK);
    } else if (current_line > 153) {
      gpu->ram->data[LY] = 0;
    } else if (current_line < 144) {
      gpu_render_scanline(gpu);
    }
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
                                   : (int16_t)ram_get(gpu->ram, tile_address);

    // if unsigned, +128 to tile number
    uint16_t tile_location =
        tile_data + ((is_unsigned << 3) ^ 128) + (tile_num << 4);

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

static void gpu_render_sprites(GPU *gpu) {}

static inline int get_color(GPU *gpu, uint8_t value, uint16_t palette) {
  return colors[((palette >> ((value << 1) | 1)) & 0x01) << 1 |
                ((palette >> (value << 1)) & 0x01)];
}
