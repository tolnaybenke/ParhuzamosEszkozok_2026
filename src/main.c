#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ocl_utils.h"

// Ez a két sor oldja meg az "undefined reference to WinMain@16" hibát Windows alatt
#define SDL_MAIN_HANDLED
#include "sdl_utils.h"

int main(int argc, char* argv[]) {
    // Megmondjuk az SDL-nek, hogy mi kezeljük a main függvényt
    SDL_SetMainReady();

    // Beállítások
    const int W = 1024;
    const int H = 1024;
    const int ITERATIONS = 5000;
    
    // true = csak mérés (gyors), false = ablakos megjelenítés
    bool benchmark_mode = false; 

    // 1. OpenCL inicializálása
    OCLResources ocl = init_opencl("kernels/life.cl", "life_step");
    
    // 2. SDL2 inicializálása
    SDLResources sdl;
    if (!benchmark_mode) {
        sdl = init_sdl(W, H);
    } else {
        // SDL inicializáció benchmark módban is kellhet az időméréshez
        SDL_Init(SDL_INIT_TIMER);
    }

    // 3. Kezdeti adatok generálása
    unsigned char* host_grid = (unsigned char*)malloc(W * H);
    for (int i = 0; i < W * H; i++) {
        host_grid[i] = (rand() % 100 < 20) ? 1 : 0;
    }

    // 4. GPU Bufferek
    cl_int err;
    cl_mem bufA = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, W * H, host_grid, &err);
    check_error(err, "bufA letrehozasa");
    cl_mem bufB = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE, W * H, NULL, &err);
    check_error(err, "bufB letrehozasa");

    // 5. Futtatás és mérés indítása
    size_t global_size[2] = { W, H };
    
    printf("Szimulacio inditasa (%dx%d, %d iteracio)...\n", W, H, ITERATIONS);

    // SDL alapú precíz időmérés
    uint64_t start_tick = SDL_GetPerformanceCounter();
    uint64_t freq = SDL_GetPerformanceFrequency();

    for (int i = 0; i < ITERATIONS; i++) {
        clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), &bufA);
        clSetKernelArg(ocl.kernel, 1, sizeof(cl_mem), &bufB);
        clSetKernelArg(ocl.kernel, 2, sizeof(int), &W);
        clSetKernelArg(ocl.kernel, 3, sizeof(int), &H);

        err = clEnqueueNDRangeKernel(ocl.queue, ocl.kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
        if(err != CL_SUCCESS) check_error(err, "Kernel futtatasa");
        
        // Ping-Pong swap
        cl_mem temp = bufA;
        bufA = bufB;
        bufB = temp;

        // Megjelenítés és eseménykezelés
        if (!benchmark_mode) {
            if (i % 2 == 0) {
                clEnqueueReadBuffer(ocl.queue, bufA, CL_TRUE, 0, W * H, host_grid, 0, NULL, NULL);
                update_display(&sdl, host_grid, W, H);
            }

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    i = ITERATIONS; // Kilépés a ciklusból
                }
            }
        }
    }
    
    clFinish(ocl.queue);
    uint64_t end_tick = SDL_GetPerformanceCounter();

    // 6. Eredmények kiszámítása
    double time_spent = (double)(end_tick - start_tick) / (double)freq;
    double cups = (double)W * H * ITERATIONS / time_spent;

    printf("\n--- Meresi eredmenyek ---\n");
    printf("Idotartam: %.4f masodperc\n", time_spent);
    printf("Teljesitmeny: %.2f MCUPS (Million Cells Updated Per Second)\n", cups / 1e6);

    // 7. Takarítás
    if (!benchmark_mode) cleanup_sdl(&sdl);
    cleanup_opencl(&ocl);
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    free(host_grid);

    return 0;
}