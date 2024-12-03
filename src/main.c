#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"
	
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#endif

////////////////////////////////////////////////////////////////////////////////
/// Declare an array of vectors/points
////////////////////////////////////////////////////////////////////////////////
const int N_POINTS = 9 * 9 * 9; // 729 punkter
vec3_t cube_points[N_POINTS]; // 9x9x9 cube
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640;

bool is_running = false;
Uint32 previous_frame_time = 0;

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

	int point_count = 0;

	// Begynne å laste vector arrayet
	// Fra -1 til 1 (i 9x9x9 kuben)
	for (float x = -1; x <= 1; x += 0.25) {
		for (float y = -1; y <= 1; y += 0.25) {
			for (float z = -1; z <= 1; z += 0.25) {
				vec3_t new_point = { .x = x, .y = y, .z = z };
				cube_points[point_count++] = new_point;
			}
		}
	}
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
	// We "lock" execution until FRAME_TARGET_TIME has passed to create a
	// consistent frame time
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

	previous_frame_time = SDL_GetTicks(); // Number of ms since game has started

	cube_rotation.x += 0.01; // Speed of rotation
	cube_rotation.y += 0.01; // Speed of rotation
	cube_rotation.z += 0.01; // Speed of rotation

	for (int i = 0; i < N_POINTS; i++) {
		vec3_t point = cube_points[i];

		// Rotate the points 0.1 degrees around y-axis for every frame update
		vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
		transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

		// Translate the points away from the camera
		transformed_point.z -= camera_position.z;

		// Projecter det nåværende punktet
		vec2_t projected_point = project(transformed_point, fov_factor);

		// Lagre den projekterte, roterte, 2D vektoren i arrayet av projekterte punkter
		projected_points[i] = projected_point;
	}
}

void render(void) {
	draw_grid();

	// Loop gjennom og rendre alle projekterte punkter
	for (int i = 0; i < N_POINTS; i++) {
		vec2_t projected_point = projected_points[i];
		draw_rect(
				projected_point.x + (window_width / 2), 
				projected_point.y + (window_height / 2), 
				4, 
				4, 
				0xFFFFFF00
		);
	}

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
