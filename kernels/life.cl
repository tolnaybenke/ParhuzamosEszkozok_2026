
__kernel void life_step(__global const uchar* current, 
                        __global uchar* next, 
                        const int width, 
                        const int height) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) return;

    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            // Torus (körbeforduló) koordináták
            int nx = (x + i + width) % width;
            int ny = (y + j + height) % height;
            neighbors += current[ny * width + nx];
        }
    }

    uchar state = current[y * width + x];
    uchar next_state = 0;

    // Game of Life szabályok
    if (state == 1) {
        if (neighbors == 2 || neighbors == 3) next_state = 1;
    } else {
        if (neighbors == 3) next_state = 1;
    }

    next[y * width + x] = next_state;
}