#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
	
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#endif

/*** data (global state) ***/

bool is_running = false;

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

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			/* For å finne posisjonen til en pixel: Finner først 
			 * antall rader fra toppen, og så teller vi kolonner fra venstre */
			color_buffer[(window_width * y) + x] = color;
		}
	}
}


void draw_grid(void) {
	for (int y = 0; y < window_height; y += 10) {
		for (int x = 0; x < window_width; x += 10) {
			color_buffer[(window_width * y) + x] = 0xFF333333; // Gray
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

void render(void) {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	draw_grid();

	render_color_buffer();

	/* Vi rensker colorbufferen før hver frame rendres */
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
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
