#include "frontend.h"

#include <SDL2/SDL.h>

void *frontend_init() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return NULL;
  }

  SDL_Window *window =
      SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    return NULL;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  return renderer;
}

void frontend_update(void *renderer, uint8_t framebuffer[160][144]) {
  // check quit
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      SDL_Quit();
      exit(0);
    }
  }
}

void frontend_draw_tiles(void *renderer, uint8_t *mem) {
  // clear screen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  SDL_RenderClear(renderer);

  for (int tile = 0; tile < 0x80; tile++) {
    for (int row = 0; row < 16; row += 2) {
      uint8_t left = mem[row + tile * 16];
      uint8_t right = mem[row + 1 + tile * 16];

      for (int col = 0; col < 8; col++) {
        uint8_t c = ((left >> col) << 1 | (right >> col)) & 0b11;
        uint8_t colors[] = {0xFF, 0xAA, 0x55, 0x00};

        SDL_SetRenderDrawColor(renderer, colors[c], colors[c], colors[c], 255);
        SDL_RenderDrawPoint(renderer, col + tile * 8, row / 2);
      }
    }
  }

  SDL_RenderPresent(renderer);
}
