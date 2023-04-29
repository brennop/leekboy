#include "frontend.h"

#include <SDL2/SDL.h>

int frontend_init() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Delay(3000);

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
