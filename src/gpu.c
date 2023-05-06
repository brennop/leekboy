#include "gpu.h"
#include "cpu.h"
#include "ram.h"
#include <stdio.h>

static void gpu_render_tiles(GPU *gpu);
static void gpu_render_sprites(GPU *gpu);

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

  if (lcdc & 0x01) {
    gpu_render_tiles(gpu);
  }

  if (lcdc & 0x02) {
    gpu_render_sprites(gpu);
  }
}

static void gpu_render_tiles(GPU *gpu) {
  uint8_t lcdc = ram_get(gpu->ram, LCDC);

  uint8_t scroll_y = ram_get(gpu->ram, SCY);
  uint8_t scroll_x = ram_get(gpu->ram, SCX);
  uint8_t window_y = ram_get(gpu->ram, WY);
  uint8_t window_x = ram_get(gpu->ram, WX) - 7;

  uint8_t window_enabled = lcdc & 0x20;
  uint8_t using_window = 0;

  if (window_enabled) {
    if (window_y <= ram_get(gpu->ram, LY)) {
      using_window = 1;
    }
  }

  uint8_t is_unsigned = lcdc & 0x10;
  uint16_t tile_data = is_unsigned ? 0x8000 : 0x8800;

  uint16_t background_memory = using_window ? (lcdc & 0x40 ? 0x9C00 : 0x9800)
                                            : (lcdc & 0x08 ? 0x9C00 : 0x9800);

  uint8_t y_pos = using_window ? ram_get(gpu->ram, LY) - window_y
                               : scroll_y + ram_get(gpu->ram, LY);

  uint16_t tile_row = (y_pos / 8) * 32;

  for (int pixel = 0; pixel < 160; pixel++) {
    uint8_t x_pos = pixel + scroll_x;

    if (using_window) {
      if (pixel >= window_x) {
        x_pos = pixel - window_x;
      }
    }

    uint16_t tile_col = x_pos / 8;

    uint16_t tile_address = background_memory + tile_row + tile_col;
    int16_t tile_num = is_unsigned ? (int8_t)ram_get(gpu->ram, tile_address)
                                   : (int16_t)ram_get(gpu->ram, tile_address);

    uint16_t tile_location =
        tile_data + (is_unsigned ? tile_num * 16 : (tile_num + 128) * 16);

    uint8_t line = (y_pos % 8) << 1;
    uint8_t data_left = ram_get(gpu->ram, tile_location + line);
    uint8_t data_right = ram_get(gpu->ram, tile_location + line + 1);

    // TODO: understand this
    int color_bit = (x_pos % 8) - 7;
    color_bit *= -1;

    // combine data
    int color_num = (data_right & (1 << color_bit)) >> color_bit;
    color_num <<= 1;
    color_num |= (data_left & (1 << color_bit)) >> color_bit;

    // TODO: get color from palette
    // for now, just use color_num as color
    int colors[] = {0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000};

    // write color to framebuffer
    int ly = ram_get(gpu->ram, LY);
    gpu->framebuffer[pixel + (ly * 160)] = colors[color_num];
  }
}

static void gpu_render_sprites(GPU *gpu) {}
