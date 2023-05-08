#include "frontend.h"
#include "ram.h"

#include <SDL2/SDL.h>

void frontend_draw_tiles(Frontend *frontend, uint8_t *mem);
void frontend_draw_sprites(Frontend *frontend, uint8_t *mem);

#define SCALE 4

void frontend_init(Frontend *frontend) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    SDL_Quit();
  }

  SDL_Window *window =
      SDL_CreateWindow("leekboy", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 160 * SCALE, 144 * SCALE, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_Texture *texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 160, 144);

  frontend->renderer = renderer;
  frontend->texture = texture;
}

void frontend_update(Frontend *frontend, Emulator *emulator) {
  uint8_t should_interrupt = 0;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      SDL_Quit();
      exit(0);
    }

    if (event.type == SDL_KEYDOWN) {
      should_interrupt = 1;
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        emulator->input.up = 1;
        break;
      case SDLK_DOWN:
        emulator->input.down = 1;
        break;
      case SDLK_LEFT:
        emulator->input.left = 1;
        break;
      case SDLK_RIGHT:
        emulator->input.right = 1;
        break;
      case SDLK_z:
        emulator->input.a = 1;
        break;
      case SDLK_x:
        emulator->input.b = 1;
        break;
      case SDLK_RETURN:
        emulator->input.start = 1;
        break;
      case SDLK_BACKSPACE:
        emulator->input.select = 1;
        break;
      }
    } else if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        emulator->input.up = 0;
        break;
      case SDLK_DOWN:
        emulator->input.down = 0;
        break;
      case SDLK_LEFT:
        emulator->input.left = 0;
        break;
      case SDLK_RIGHT:
        emulator->input.right = 0;
        break;
      case SDLK_z:
        emulator->input.a = 0;
        break;
      case SDLK_x:
        emulator->input.b = 0;
        break;
      case SDLK_RETURN:
        emulator->input.start = 0;
        break;
      case SDLK_BACKSPACE:
        emulator->input.select = 0;
        break;
      }
    }
  }

  if (should_interrupt)
    cpu_interrupt(&emulator->cpu, INT_JOYPAD);

  // draw
  SDL_SetRenderDrawColor(frontend->renderer, 0, 0, 0, 255);
  SDL_RenderClear(frontend->renderer);

  // Rect
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = 160 * SCALE;
  rect.h = 144 * SCALE;

  SDL_UpdateTexture(frontend->texture, NULL, emulator->gpu.framebuffer,
                    160 * sizeof(uint32_t));
  SDL_RenderCopy(frontend->renderer, frontend->texture, NULL, &rect);

  /* frontend_draw_tiles(frontend, emulator->cpu.ram->data + 0x8000); */
  /* frontend_draw_sprites(frontend, emulator->cpu.ram->data); */

  SDL_RenderPresent(frontend->renderer);
}

void frontend_run(Frontend *frontend, Emulator *emulator) {
  while (1) {
    frontend_update(frontend, emulator);
    emulator_step(emulator);
  }
}

void frontend_draw_tiles(Frontend *frontend, uint8_t *mem) {
  int xOffset = 160 * 2 + 8;
  int tilesPerRow = 16;
  for (int tile = 0; tile < 0x100; tile++) {
    for (int row = 0; row < 16; row += 2) {
      uint8_t left = mem[row + tile * 16];
      uint8_t right = mem[row + 1 + tile * 16];

      for (int col = 0; col < 8; col++) {
        uint8_t c = ((left >> col) << 1 | (right >> col)) & 0b11;
        uint8_t colors[] = {0xFF, 0xAA, 0x55, 0x00};

        SDL_SetRenderDrawColor(frontend->renderer, colors[c], colors[c],
                               colors[c], 255);
        SDL_RenderDrawPoint(frontend->renderer,
                            xOffset + col + tile % tilesPerRow * 8,
                            tile / tilesPerRow * 8 + row / 2);
      }
    }
  }

  SDL_RenderPresent(frontend->renderer);
}

void frontend_draw_sprites(Frontend *frontend, uint8_t *mem) {
  int xOffset = 160 * 2 + 8;
  int yOffset = 144 * 2 + 8;
  int spritesPerRow = 10;

  for (int sprite = 0; sprite < 40; sprite++) {
    uint8_t tile = mem[0xfe00 + sprite * 4 + 2];

    for (int row = 0; row < 8; row += 2) {
      uint8_t left = mem[row + tile * 16 + 0x8000];
      uint8_t right = mem[row + 1 + tile * 16 + 0x8000];

      for (int col = 0; col < 8; col++) {
        uint8_t c = ((left >> col) << 1 | (right >> col)) & 0b11;
        uint8_t colors[] = {0xFF, 0xAA, 0x55, 0x00};

        SDL_SetRenderDrawColor(frontend->renderer, colors[c], colors[c],
                               colors[c], 255);
        SDL_RenderDrawPoint(frontend->renderer,
                            xOffset + col + sprite % spritesPerRow * 8,
                            yOffset + sprite / spritesPerRow * 8 + row / 2);
      }
    }
  }
}
