#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixel_buffer; // Itt tároljuk az ARGB színeket a megjelenítéshez
} SDLResources;

SDLResources init_sdl(int width, int height);
void update_display(SDLResources* sdl, const unsigned char* grid, int width, int height);
void cleanup_sdl(SDLResources* sdl);

#endif