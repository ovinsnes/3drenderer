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
		SDL_WINDOW_RESIZABLE
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
	SDL_SetWindowFullscreen(window, 0); // "0" or "SDL_WINDOW_FULLSCREEN"

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

////////////////////////////////////////////////////////////////////////////////
/// Digital differential analyzer (DDA) algorithm for line rasterization
////////////////////////////////////////////////////////////////////////////////
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);
	
	// If dy > dx, we have a m > 1, and need to run the side length of dy
	// instead of dx. This fixes line gaps for steep lines.
	int longest_side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

	// Find how much we need to increment both x and y each step
	float x_inc = delta_x / (float)longest_side_length;
	float y_inc = delta_y / (float)longest_side_length;

	float current_x = x0;
	float current_y = y0;

	for (int i = 0; i <= longest_side_length; i++) {
		draw_pixel(round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
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

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
