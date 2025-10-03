all: build

build:
	gcc -Wall -std=c99 -arch arm64 src/*.c \
	 -I/opt/homebrew/opt/sdl2/include/SDL2 -I/opt/homebrew/opt/sdl2_ttf/include/SDL2 -D_REENTRANT \
	-L/opt/homebrew/opt/sdl2/lib -L/opt/homebrew/opt/sdl2_ttf/lib -lSDL2 -lSDL2_ttf -o renderer

run:
	./renderer

clean:
	rm renderer
