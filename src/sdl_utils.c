#include "sdl_utils.h"

SDLResources init_sdl(int width, int height) {
    SDLResources res;
    SDL_Init(SDL_INIT_VIDEO);
    res.window = SDL_CreateWindow("OpenCL Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    res.renderer = SDL_CreateRenderer(res.window, -1, SDL_RENDERER_ACCELERATED);
    res.texture = SDL_CreateTexture(res.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    res.pixel_buffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    return res;
}

void update_display(SDLResources* sdl, const unsigned char* grid, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        // 1 = Fehér, 0 = Fekete
        sdl->pixel_buffer[i] = (grid[i] == 1) ? 0xFFFFFFFF : 0xFF000000;
    }
    SDL_UpdateTexture(sdl->texture, NULL, sdl->pixel_buffer, width * sizeof(uint32_t));
    SDL_RenderClear(sdl->renderer);
    SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, NULL);
    SDL_RenderPresent(sdl->renderer);
}

void cleanup_sdl(SDLResources* sdl) {
    free(sdl->pixel_buffer);
    SDL_DestroyTexture(sdl->texture);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}