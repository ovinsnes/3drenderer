#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
	
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#endif

bool is_running = false;

void setup(void) {
	// Alloker nødvendig antall bytes i minnet for fargebuffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// Lage en SDL texture for å vise fargebufferet
	color_buffer_texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			window_width,
			window_height
	);
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;
			break;
	}
}

void update(void) {
	// TODO:
}

void render(void) {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	draw_grid();

	draw_pixel(20, 20, 0xFFFF00FF);
	draw_rect(300, 150, 200, 100, 0xFFFFCCCC);

	render_color_buffer();

	/* Vi rensker colorbufferen før hver frame rendres */
	clear_color_buffer(0xFF000000); // Svart

	SDL_RenderPresent(renderer);
}

int main(void) {
	
	initialize_window();

	is_running = initialize_window();

	setup();

	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();
	
    return 0;
}
