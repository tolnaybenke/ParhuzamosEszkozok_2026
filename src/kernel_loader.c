#include "kernel_loader.h"
#include <stdio.h>
#include <stdlib.h>

char* load_kernel_source(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { 
        fprintf(stderr, "Hiba: A %s kernel fajl nem talalhato!\n", filename); 
        exit(1); 
    }
    
    // Fájl méretének meghatározása
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    
    // Memória foglalása a stringnek (+1 a lezáró \0 karakternek)
    char* source = (char*)malloc(size + 1);
    if (!source) {
        fprintf(stderr, "Hiba: Nem sikerult memoriat foglalni a kernel beolvasasahoz!\n");
        fclose(fp);
        exit(1);
    }
    
    // Beolvasás és lezárás
    fread(source, 1, size, fp);
    source[size] = '\0';
    
    fclose(fp);
    return source;
}