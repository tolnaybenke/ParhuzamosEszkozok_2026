#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ocl_utils.h"

#define SDL_MAIN_HANDLED
#include "sdl_utils.h"

// CPU-s szekvenciális implementáció a mérésekhez
void cpu_life_step(const unsigned char* current, unsigned char* next, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int neighbors = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) continue;
                    int nx = (x + j + width) % width;
                    int ny = (y + i + height) % height;
                    neighbors += current[ny * width + nx];
                }
            }
            unsigned char state = current[y * width + x];
            if (state == 1 && (neighbors == 2 || neighbors == 3)) next[y * width + x] = 1;
            else if (state == 0 && neighbors == 3) next[y * width + x] = 1;
            else next[y * width + x] = 0;
        }
    }
}

// 1. SVG Gráf: Gyorsulás
void export_svg(const char* filename, const char* title, const char* x_label, const char* y_label, int* x_data, double* y_data, int count, const char* color) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    int width = 800, height = 400, pad_x = 90, pad_y = 60; // Növelt pad_x a feliratoknak
    double max_y = 0.0001;
    for (int i = 0; i < count; i++) if (y_data[i] > max_y) max_y = y_data[i];
    max_y *= 1.2; 

    fprintf(fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\">\n", width, height);
    fprintf(fp, "<rect width=\"100%%\" height=\"100%%\" fill=\"#f8f9fa\" rx=\"10\"/>\n");
    fprintf(fp, "<text x=\"%d\" y=\"30\" font-family=\"Arial\" font-size=\"20\" font-weight=\"bold\" text-anchor=\"middle\">%s</text>\n", width/2, title);
    
    // X Tengely címke (alul)
    fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" fill=\"#555\">%s</text>\n", width/2, height - 15, x_label);
    
    // Y Tengely címke (bal oldalon, elforgatva)
    fprintf(fp, "<text x=\"25\" y=\"%d\" transform=\"rotate(-90, 25, %d)\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" fill=\"#555\">%s</text>\n", height/2, height/2, y_label);

    // Vízszintes segédvonalak (Grid) és Y értékek felirata
    int num_ticks = 5;
    for (int i = 0; i <= num_ticks; i++) {
        double tick_val = max_y * i / (double)num_ticks;
        int cy = height - pad_y - (int)((tick_val / max_y) * (height - 2 * pad_y));
        
        // Szaggatott segédvonal
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#ddd\" stroke-dasharray=\"4,4\" stroke-width=\"1\"/>\n", pad_x, cy, width - pad_x + 20, cy);
        // Y érték felirat
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"end\" fill=\"#555\">%.1f</text>\n", pad_x - 10, cy + 4, tick_val);
    }

    // Fő tengelyek (X, Y vonalak)
    fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#333\" stroke-width=\"2\"/>\n", pad_x, height-pad_y, width-pad_x+20, height-pad_y);
    fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#333\" stroke-width=\"2\"/>\n", pad_x, height-pad_y, pad_x, pad_y-20);

    // Vonal rajzolása
    fprintf(fp, "<polyline fill=\"none\" stroke=\"%s\" stroke-width=\"3\" points=\"", color);
    for(int i = 0; i < count; i++) {
        int cx = pad_x + (i * (width - 2 * pad_x) / (count - 1));
        int cy = height - pad_y - (int)((y_data[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "%d,%d ", cx, cy);
    }
    fprintf(fp, "\"/>\n");

    // Pontok
    for(int i = 0; i < count; i++) {
        int cx = pad_x + (i * (width - 2 * pad_x) / (count - 1));
        int cy = height - pad_y - (int)((y_data[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"5\" fill=\"%s\"/>\n", cx, cy, color);
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"middle\">%d</text>\n", cx, height - pad_y + 20, x_data[i]);
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" font-weight=\"bold\" text-anchor=\"middle\" dy=\"-15\">%.1f</text>\n", cx, cy, y_data[i]);
    }
    fprintf(fp, "</svg>\n");
    fclose(fp);
}

// 2. SVG Gráf: Összehasonlítás
void export_svg_compare(const char* filename, const char* title, const char* x_label, const char* y_label,
                        int* x_data, double* y_data1, double* y_data2, int count, 
                        const char* name1, const char* name2, const char* color1, const char* color2) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;
    int width = 800, height = 400, pad_x = 90, pad_y = 60;
    
    double max_y = 0.0001;
    for (int i = 0; i < count; i++) {
        if (y_data1[i] > max_y) max_y = y_data1[i];
        if (y_data2[i] > max_y) max_y = y_data2[i];
    }
    max_y *= 1.2; 

    fprintf(fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\">\n", width, height);
    fprintf(fp, "<rect width=\"100%%\" height=\"100%%\" fill=\"#f8f9fa\" rx=\"10\"/>\n");
    fprintf(fp, "<text x=\"%d\" y=\"30\" font-family=\"Arial\" font-size=\"20\" font-weight=\"bold\" text-anchor=\"middle\">%s</text>\n", width/2, title);
    
    // X Tengely címke
    fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" fill=\"#555\">%s</text>\n", width/2, height - 15, x_label);
    
    // Y Tengely címke (elforgatva)
    fprintf(fp, "<text x=\"25\" y=\"%d\" transform=\"rotate(-90, 25, %d)\" font-family=\"Arial\" font-size=\"14\" text-anchor=\"middle\" fill=\"#555\">%s</text>\n", height/2, height/2, y_label);

    // Vízszintes segédvonalak (Grid) és Y értékek felirata
    int num_ticks = 5;
    for (int i = 0; i <= num_ticks; i++) {
        double tick_val = max_y * i / (double)num_ticks;
        int cy = height - pad_y - (int)((tick_val / max_y) * (height - 2 * pad_y));
        fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#ddd\" stroke-dasharray=\"4,4\" stroke-width=\"1\"/>\n", pad_x, cy, width - pad_x + 20, cy);
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"end\" fill=\"#555\">%.1f</text>\n", pad_x - 10, cy + 4, tick_val);
    }
    
    // Fő tengelyek
    fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#333\" stroke-width=\"2\"/>\n", pad_x, height-pad_y, width-pad_x+20, height-pad_y);
    fprintf(fp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#333\" stroke-width=\"2\"/>\n", pad_x, height-pad_y, pad_x, pad_y-20);

    // Vonal 1
    fprintf(fp, "<polyline fill=\"none\" stroke=\"%s\" stroke-width=\"3\" points=\"", color1);
    for(int i = 0; i < count; i++) {
        int cx = pad_x + (i * (width - 2 * pad_x) / (count - 1));
        int cy = height - pad_y - (int)((y_data1[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "%d,%d ", cx, cy);
    }
    fprintf(fp, "\"/>\n");

    // Vonal 2
    fprintf(fp, "<polyline fill=\"none\" stroke=\"%s\" stroke-width=\"3\" points=\"", color2);
    for(int i = 0; i < count; i++) {
        int cx = pad_x + (i * (width - 2 * pad_x) / (count - 1));
        int cy = height - pad_y - (int)((y_data2[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "%d,%d ", cx, cy);
    }
    fprintf(fp, "\"/>\n");

    // Pontok
    for(int i = 0; i < count; i++) {
        int cx = pad_x + (i * (width - 2 * pad_x) / (count - 1));
        
        int cy1 = height - pad_y - (int)((y_data1[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"5\" fill=\"%s\"/>\n", cx, cy1, color1);
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" font-weight=\"bold\" fill=\"%s\" text-anchor=\"middle\" dy=\"15\">%.2f</text>\n", cx, cy1, color1, y_data1[i]);
        
        int cy2 = height - pad_y - (int)((y_data2[i] / max_y) * (height - 2 * pad_y));
        fprintf(fp, "<circle cx=\"%d\" cy=\"%d\" r=\"5\" fill=\"%s\"/>\n", cx, cy2, color2);
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" font-weight=\"bold\" fill=\"%s\" text-anchor=\"middle\" dy=\"-10\">%.2f</text>\n", cx, cy2, color2, y_data2[i]);
        
        fprintf(fp, "<text x=\"%d\" y=\"%d\" font-family=\"Arial\" font-size=\"12\" text-anchor=\"middle\">%d</text>\n", cx, height - pad_y + 20, x_data[i]);
    }

    // Legend
    fprintf(fp, "<rect x=\"%d\" y=\"50\" width=\"120\" height=\"60\" fill=\"white\" stroke=\"#ccc\" rx=\"5\"/>\n", pad_x + 20);
    fprintf(fp, "<circle cx=\"%d\" cy=\"70\" r=\"5\" fill=\"%s\"/><text x=\"%d\" y=\"75\" font-family=\"Arial\" font-size=\"14\">%s</text>\n", pad_x + 35, color1, pad_x + 50, name1);
    fprintf(fp, "<circle cx=\"%d\" cy=\"95\" r=\"5\" fill=\"%s\"/><text x=\"%d\" y=\"100\" font-family=\"Arial\" font-size=\"14\">%s</text>\n", pad_x + 35, color2, pad_x + 50, name2);

    fprintf(fp, "</svg>\n");
    fclose(fp);
}

int main(int argc, char* argv[]) {
    SDL_SetMainReady();
    bool benchmark_mode = true; 

    OCLResources ocl = init_opencl("kernels/life.cl", "life_step");

    if (benchmark_mode) {
        SDL_Init(SDL_INIT_TIMER);
        uint64_t freq = SDL_GetPerformanceFrequency();
        uint64_t start_tick, end_tick;

        // 1. TESZT: Méret skálázódása
        int fixed_iters = 500;
        int sizes[] = {32, 64, 128, 256, 512, 1024, 2048};
        int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

        double size_speedups[7];
        double size_cpu_mcups[7], size_gpu_mcups[7];
        double size_cpu_time[7], size_gpu_time[7];

        printf("\n=== 1. TESZT: RACS MERET SKALAZODASA (Fix %d iteracio) ===\n", fixed_iters);
        printf("| Meret     | CPU Ido    | GPU Ido    |Gyorsulas| CPU MCUPS| GPU MCUPS|\n");
        printf("|-----------|------------|------------|----------|----------|----------|\n");
        
        for (int i = 0; i < num_sizes; i++) {
            int W = sizes[i], H = sizes[i];
            size_t mem_size = W * H * sizeof(unsigned char);
            unsigned char* init_grid = (unsigned char*)malloc(mem_size);
            unsigned char* cpu_gridA = (unsigned char*)malloc(mem_size);
            unsigned char* cpu_gridB = (unsigned char*)malloc(mem_size);
            for (int j = 0; j < W * H; j++) {
                init_grid[j] = (rand() % 100 < 20) ? 1 : 0;
                cpu_gridA[j] = init_grid[j];
            }

            // CPU mérés
            start_tick = SDL_GetPerformanceCounter();
            unsigned char* curr = cpu_gridA, *next = cpu_gridB;
            for (int j = 0; j < fixed_iters; j++) {
                cpu_life_step(curr, next, W, H);
                unsigned char* temp = curr; curr = next; next = temp;
            }
            end_tick = SDL_GetPerformanceCounter();
            double cpu_time = (double)(end_tick - start_tick) / freq;

            // GPU mérés
            cl_int err;
            cl_mem bufA = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, mem_size, init_grid, &err);
            cl_mem bufB = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE, mem_size, NULL, &err);
            size_t global_size[2] = { W, H };

            start_tick = SDL_GetPerformanceCounter();
            for (int j = 0; j < fixed_iters; j++) {
                clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), &bufA);
                clSetKernelArg(ocl.kernel, 1, sizeof(cl_mem), &bufB);
                clSetKernelArg(ocl.kernel, 2, sizeof(int), &W);
                clSetKernelArg(ocl.kernel, 3, sizeof(int), &H);
                clEnqueueNDRangeKernel(ocl.queue, ocl.kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
                cl_mem temp = bufA; bufA = bufB; bufB = temp;
            }
            clFinish(ocl.queue);
            end_tick = SDL_GetPerformanceCounter();
            double gpu_time = (double)(end_tick - start_tick) / freq;

            size_speedups[i] = cpu_time / gpu_time;
            size_gpu_mcups[i] = ((double)W * H * fixed_iters / gpu_time) / 1000000.0;
            size_cpu_mcups[i] = ((double)W * H * fixed_iters / cpu_time) / 1000000.0;
            size_cpu_time[i] = cpu_time; 
            size_gpu_time[i] = gpu_time;

            printf("| %4dx%-4d | %8.4f s | %8.4f s | %6.2fx | %8.1f | %8.1f |\n", 
                    W, H, cpu_time, gpu_time, size_speedups[i], size_cpu_mcups[i], size_gpu_mcups[i]);

            clReleaseMemObject(bufA); clReleaseMemObject(bufB);
            free(init_grid); free(cpu_gridA); free(cpu_gridB);
        }

        // 2. TESZT: Iterációszám skálázódása
        int fixed_W = 1024, fixed_H = 1024;
        int iter_counts[] = {10, 50, 100, 500, 1000, 2000};
        int num_iters = sizeof(iter_counts) / sizeof(iter_counts[0]);

        double iter_speedups[6];
        double iter_cpu_mcups[6], iter_gpu_mcups[6];
        double iter_cpu_time[6], iter_gpu_time[6]; 

        printf("\n=== 2. TESZT: ITERACIOSZAM SKALAZODASA (Fix %dx%d racs) ===\n", fixed_W, fixed_H);
        printf("| Iteracio  | CPU Ido    | GPU Ido    |Gyorsulas| CPU MCUPS| GPU MCUPS|\n");
        printf("|-----------|------------|------------|----------|----------|----------|\n");
        
        size_t mem_size = fixed_W * fixed_H * sizeof(unsigned char);
        for (int i = 0; i < num_iters; i++) {
            int current_iters = iter_counts[i];
            unsigned char* init_grid = (unsigned char*)malloc(mem_size);
            unsigned char* cpu_gridA = (unsigned char*)malloc(mem_size);
            unsigned char* cpu_gridB = (unsigned char*)malloc(mem_size);
            for (int j = 0; j < fixed_W * fixed_H; j++) {
                init_grid[j] = (rand() % 100 < 20) ? 1 : 0;
                cpu_gridA[j] = init_grid[j];
            }

            start_tick = SDL_GetPerformanceCounter();
            unsigned char* curr = cpu_gridA, *next = cpu_gridB;
            for (int j = 0; j < current_iters; j++) {
                cpu_life_step(curr, next, fixed_W, fixed_H);
                unsigned char* temp = curr; curr = next; next = temp;
            }
            end_tick = SDL_GetPerformanceCounter();
            double cpu_time = (double)(end_tick - start_tick) / freq;

            cl_int err;
            cl_mem bufA = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, mem_size, init_grid, &err);
            cl_mem bufB = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE, mem_size, NULL, &err);
            size_t global_size[2] = { fixed_W, fixed_H };

            start_tick = SDL_GetPerformanceCounter();
            for (int j = 0; j < current_iters; j++) {
                clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), &bufA);
                clSetKernelArg(ocl.kernel, 1, sizeof(cl_mem), &bufB);
                clSetKernelArg(ocl.kernel, 2, sizeof(int), &fixed_W);
                clSetKernelArg(ocl.kernel, 3, sizeof(int), &fixed_H);
                clEnqueueNDRangeKernel(ocl.queue, ocl.kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
                cl_mem temp = bufA; bufA = bufB; bufB = temp;
            }
            clFinish(ocl.queue);
            end_tick = SDL_GetPerformanceCounter();
            double gpu_time = (double)(end_tick - start_tick) / freq;

            iter_speedups[i] = cpu_time / gpu_time;
            iter_gpu_mcups[i] = ((double)fixed_W * fixed_H * current_iters / gpu_time) / 1000000.0;
            iter_cpu_mcups[i] = ((double)fixed_W * fixed_H * current_iters / cpu_time) / 1000000.0;
            iter_cpu_time[i] = cpu_time; 
            iter_gpu_time[i] = gpu_time; 

            printf("| %-9d | %8.4f s | %8.4f s | %6.2fx | %8.1f | %8.1f |\n", 
                    current_iters, cpu_time, gpu_time, iter_speedups[i], iter_cpu_mcups[i], iter_gpu_mcups[i]);

            clReleaseMemObject(bufA); clReleaseMemObject(bufB);
            free(init_grid); free(cpu_gridA); free(cpu_gridB);
        }

        // === GRAFIKONOK GENERÁLÁSA (Frissített paraméterekkel) ===
        printf("\nGrafikonok keszitese SVG formatumban...\n");
        
        // 1. Gyorsulás grafikonok
        export_svg("01_meret_speedup.svg", "GPU Gyorsulas a racsmeret fuggvenyeben", "Racs merete", "Gyorsulas (x-szeres)", sizes, size_speedups, num_sizes, "#2ecc71");
        export_svg("02_iteracio_speedup.svg", "GPU Gyorsulas az iteracioszam fuggvenyeben", "Iteraciok szama", "Gyorsulas (x-szeres)", iter_counts, iter_speedups, num_iters, "#2ecc71");
        
        // 2. Teljesítmény grafikonok
        export_svg_compare("03_meret_mcups.svg", "CPU vs GPU Teljesitmeny (Racs meret)", "Racs merete", "Teljesitmeny (MCUPS)",
                           sizes, size_cpu_mcups, size_gpu_mcups, num_sizes, "CPU", "GPU", "#e74c3c", "#3498db");
        export_svg_compare("04_iteracio_mcups.svg", "CPU vs GPU Teljesitmeny (Iteracioszam)", "Iteraciok szama", "Teljesitmeny (MCUPS)",
                           iter_counts, iter_cpu_mcups, iter_gpu_mcups, num_iters, "CPU", "GPU", "#e74c3c", "#3498db");

        // 3. Futási idő grafikonok
        export_svg_compare("05_meret_time.svg", "CPU vs GPU Futasi Ido (Racs meret)", "Racs merete", "Ido (masodperc)",
                           sizes, size_cpu_time, size_gpu_time, num_sizes, "CPU (s)", "GPU (s)", "#e74c3c", "#3498db");
        export_svg_compare("06_iteracio_time.svg", "CPU vs GPU Futasi Ido (Iteracioszam)", "Iteraciok szama", "Ido (masodperc)",
                           iter_counts, iter_cpu_time, iter_gpu_time, num_iters, "CPU (s)", "GPU (s)", "#e74c3c", "#3498db");
        
        printf("\nKesz! Nyisd meg a generalt 6 darab SVG fajlt.\n");
    } 
    else {
        // --- GRAFIKUS MÓD ---
        const int W = 1024, H = 1024;
        SDLResources sdl = init_sdl(W, H);

        unsigned char* host_grid = (unsigned char*)malloc(W * H);
        for (int i = 0; i < W * H; i++) host_grid[i] = (rand() % 100 < 20) ? 1 : 0;

        cl_int err;
        cl_mem bufA = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, W * H, host_grid, &err);
        cl_mem bufB = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE, W * H, NULL, &err);

        size_t global_size[2] = { W, H };
        printf("Grafikus mod elinditva (%dx%d)...\n", W, H);
        
        int frame = 0; bool running = true;
        while (running) {
            clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), &bufA);
            clSetKernelArg(ocl.kernel, 1, sizeof(cl_mem), &bufB);
            clSetKernelArg(ocl.kernel, 2, sizeof(int), &W);
            clSetKernelArg(ocl.kernel, 3, sizeof(int), &H);

            clEnqueueNDRangeKernel(ocl.queue, ocl.kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
            cl_mem temp = bufA; bufA = bufB; bufB = temp;

            if (frame % 2 == 0) {
                clEnqueueReadBuffer(ocl.queue, bufA, CL_TRUE, 0, W * H, host_grid, 0, NULL, NULL);
                update_display(&sdl, host_grid, W, H);
            }

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
            }
            frame++;
        }
        clReleaseMemObject(bufA); clReleaseMemObject(bufB);
        free(host_grid); cleanup_sdl(&sdl);
    }

    cleanup_opencl(&ocl);
    return 0;
}