#ifndef KERNEL_LOADER_H
#define KERNEL_LOADER_H

// Beolvassa a megadott fájlt és visszatér a tartalmával (egy null-terminált stringgel).
// A memóriafelszabadítás (free) a hívó felelőssége!
char* load_kernel_source(const char* filename);

#endif