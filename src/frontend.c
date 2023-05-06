#include "frontend.h"
#include "ram.h"

#include <SDL2/SDL.h>

void frontend_init(Frontend *frontend) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    SDL_Quit();
  }

  SDL_Window *window =
      SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

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

  frontend->input = (Input){0, 0, 0, 0, 0, 0, 0, 0};
}

void frontend_update(Frontend *frontend, int framebuffer[160 * 144], CPU *cpu) {
  uint8_t should_interrupt = 0;

  // input
  uint8_t joypad = (~ram_get(cpu->ram, RAM_JOYP)) & 0b00110000;
  if (joypad & 0x10) {
    if (frontend->input.right) joypad |= 0b1110;
    if (frontend->input.left) joypad |= 0b1101;
    if (frontend->input.up) joypad |= 0b1011;
    if (frontend->input.down) joypad |= 0b0111;
  }
  if (joypad & 0x20) {
    if (frontend->input.a) joypad |= 0b1110;
    if (frontend->input.b) joypad |= 0b1101;
    if (frontend->input.select) joypad |= 0b1011;
    if (frontend->input.start) joypad |= 0b0111;
  }
  ram_set(cpu->ram, RAM_JOYP, ~joypad & 0x3F);
  printf("ram[0xFF00] = %02X\n", ram_get(cpu->ram, RAM_JOYP));

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      SDL_Quit();
      exit(0);
    }

    if (event.type == SDL_KEYDOWN) {
      should_interrupt = 1;
      switch (event.key.keysym.sym) {
      case SDLK_UP:        frontend->input.up = 1; break;
      case SDLK_DOWN:      frontend->input.down = 1; break;
      case SDLK_LEFT:      frontend->input.left = 1; break;
      case SDLK_RIGHT:     frontend->input.right = 1; break;
      case SDLK_z:         frontend->input.a = 1; break;
      case SDLK_x:         frontend->input.b = 1; break;
      case SDLK_RETURN:    frontend->input.start = 1; break;
      case SDLK_BACKSPACE: frontend->input.select = 1; break;
      }
    }
    else if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
      case SDLK_UP:        frontend->input.up = 0; break;
      case SDLK_DOWN:      frontend->input.down = 0; break;
      case SDLK_LEFT:      frontend->input.left = 0; break;
      case SDLK_RIGHT:     frontend->input.right = 0; break;
      case SDLK_z:         frontend->input.a = 0; break;
      case SDLK_x:         frontend->input.b = 0; break;
      case SDLK_RETURN:    frontend->input.start = 0; break;
      case SDLK_BACKSPACE: frontend->input.select = 0; break;
      }
    }
  }

  if (should_interrupt) cpu_interrupt(cpu, INT_JOYPAD);

  // draw
  SDL_SetRenderDrawColor(frontend->renderer, 0, 0, 0, 255);
  SDL_RenderClear(frontend->renderer);

  // Rect
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = 160 * 2;
  rect.h = 144 * 2;

  SDL_UpdateTexture(frontend->texture, NULL, framebuffer,
                    160 * sizeof(uint32_t));
  SDL_RenderCopy(frontend->renderer, frontend->texture, NULL, &rect);

  frontend_draw_tiles(frontend, cpu->ram->data + 0x8000);

  SDL_RenderPresent(frontend->renderer);
}

void frontend_draw_tiles(Frontend * frontend, uint8_t * mem) {
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
