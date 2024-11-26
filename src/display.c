#include <stdio.h>
#include "display.h"

SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;
/* Vi bruker 32bytes fixed size unsigned int for å angi farge */
uint32_t* color_buffer = NULL; 
SDL_Texture* color_buffer_texture = NULL;	// SDL Texture som brukes til å vise innholdet i fargebufferet
int window_width = 800;
int window_height = 600;


bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	// Bruke SDL til å spørre hva er fullscreen max bredde og høyde på monitor
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;
	
	// Create a SDL window
	window = SDL_CreateWindow(
		NULL, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}

	// Create a SDL renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	return true;
}

void draw_grid(void) {
	for (int y = 0; y < window_height; y += 10) {
		for (int x = 0; x < window_width; x += 10) {
			color_buffer[(window_width * y) + x] = 0xFF333333; // Gray
		}
	}
}

void draw_pixel(int x, int y, uint32_t color) {
	if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
		color_buffer[(window_width * y) + x] = color;
	}
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
	for (int col = y; col < y + height; col++) {
		for (int row = x; row < x + width; row++) {
			draw_pixel(row, col, color);
		}
	}
}

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			/* For å finne posisjonen til en pixel: Finner først 
			 * antall rader fra toppen, og så teller vi kolonner fra venstre */
			color_buffer[(window_width * y) + x] = color;
		}
	}
}



void render_color_buffer(void) {
	// Oppdatere texturen med ny pixel data (fargebufferet)
	SDL_UpdateTexture(
			color_buffer_texture,
			NULL,
			color_buffer,
			(int)(window_width * sizeof(uint32_t))
	);
	// Kopiere texturen til renderen
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
