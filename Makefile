all: build

# Debug build (med symboler, uten optimalisering)
CFLAGS_DEBUG = -Wall -std=c99 -g -O0 -arch arm64

# Release build (optimalisert)
CFLAGS_RELEASE = -Wall -std=c99 -O3 -march=native -ffast-math -arch arm64

# Default til release for beste ytelse
CFLAGS = $(CFLAGS_RELEASE)

build:
	gcc $(CFLAGS) src/*.c \
	 -I/opt/homebrew/opt/sdl2/include/SDL2 -I/opt/homebrew/opt/sdl2_ttf/include/SDL2 -D_REENTRANT \
	-L/opt/homebrew/opt/sdl2/lib -L/opt/homebrew/opt/sdl2_ttf/lib -lSDL2 -lSDL2_ttf -o renderer

debug:
	gcc $(CFLAGS_DEBUG) src/*.c \
	 -I/opt/homebrew/opt/sdl2/include/SDL2 -I/opt/homebrew/opt/sdl2_ttf/include/SDL2 -D_REENTRANT \
	-L/opt/homebrew/opt/sdl2/lib -L/opt/homebrew/opt/sdl2_ttf/lib -lSDL2 -lSDL2_ttf -o renderer

run:
	./renderer

clean:
	rm -rf renderer *.dSYM
