all: build

build:
	gcc -Wall -std=c99 -arch arm64 src/main.c \
	 -I/opt/homebrew/opt/sdl2/include -D_REENTRANT \
	-L/opt/homebrew/opt/sdl2/lib -lSDL2 -o renderer

run:
	./renderer

clean:
	rm renderer
