CC = gcc
CFLAGS = -Iinclude -Wall -O3 -std=gnu99
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lOpenCL -lm

SRC = src/main.c src/ocl_utils.c src/sdl_utils.c
OBJ = $(SRC:.c=.o)
TARGET = gol_opencl.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)