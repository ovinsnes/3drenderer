#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <time.h>

#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
	
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Array of triangles that should be rendered frame by frame
////////////////////////////////////////////////////////////////////////////////
triangle_t* triangles_to_render = NULL;

////////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
////////////////////////////////////////////////////////////////////////////////
bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
float fov_factor = 640;

////////////////////////////////////////////////////////////////////////////////
// Setup function to initialize variables and game objects
////////////////////////////////////////////////////////////////////////////////
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

	// Loads the cube values in the mesh data structure
	//load_cube_mesh_data();

	clock_t start, end;
	double cpu_time_used;

	start = clock();
	load_obj_file_data("./assets/diamond.obj");
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("load_obj_data() took %f seconds to execute\n", cpu_time_used);
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
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	// Only delay execution if we are running too fast
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks(); // Number of ms since game has started
	
	// Initialize the array of triangles to render
	triangles_to_render = NULL;

	mesh.rotation.x += 0.01; // Speed of rotation
	mesh.rotation.y += 0.01; // Speed of rotation
	mesh.rotation.z += 0.01; // Speed of rotation

	// Loop all triangle faces of out mesh
	int num_faces = array_length(mesh.faces);

	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		// Array starts at index 0 so we compensate by subtracting 1
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];

		// Loop all 3 vertices of current face and apply *transformations*
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			// Rotate the points 0.1 degrees around y-axis for every frame update
			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// Translate the vertex away from the camera in the z-plane
			transformed_vertex.z += 5;

			// Lagre punktene i array for transformerte punkter
			transformed_vertices[j] = transformed_vertex;
		}

		triangle_t projected_triangle;

		// TODO: Backface culling: Sjekk om flaten skal vises eller gjemmes bort
		// Finn BA: b1-a1, b2-a2, b3-a3
		vec3_t vec_BA = {
			.x = transformed_vertices[1].x - transformed_vertices[0].x,
			.y = transformed_vertices[1].y - transformed_vertices[0].y,
			.z = transformed_vertices[1].z - transformed_vertices[0].z
		};
		// Finn CA: c1-a1, c2-a2, c3-a3
		vec3_t vec_CA = {
			.x = transformed_vertices[2].x - transformed_vertices[0].x,
			.y = transformed_vertices[2].y - transformed_vertices[0].y,
			.z = transformed_vertices[2].z - transformed_vertices[0].z
		};
		vec3_normalize(&vec_BA);
		vec3_normalize(&vec_CA);

		// Finn kryssproduktet mellom dem (venstre-hånd koordinatsystem)
		vec3_t normal = vec3_cross(vec_BA, vec_CA);
		// Normaliser normalen for planet
		vec3_normalize(&normal);

		// Finn kamera strålevektoren
		vec3_t camera_ray = vec3_sub(camera_position, transformed_vertices[0]);

		// Punktproduktet mellom kamerastrålen og normalen N
		float dot = vec3_dot(normal, camera_ray);

		bool apply_culling = true;

		// Sjekk om flaten skal vises eller gjemmes bort
		if (dot <= 0 && apply_culling) {
			continue;
		}

		// Loop gjennom alle 3 punktene av den nåværende flaten og *projiser* dem
		for (int j = 0; j < 3; j++) {
			// Project the current vertex onto 2D space
			vec2_t projected_point = project(transformed_vertices[j], fov_factor);

			// Scale and translate the projected points to the middle of the screen
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;
		}

		// Save the transformed and projected triangle in the array of triangles to render
		array_push(triangles_to_render, projected_triangle);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
////////////////////////////////////////////////////////////////////////////////
void render(void) {
	draw_grid();

	// Loop gjennom og rendre alle projekterte triangler (faces)
	int num_triangles = array_length(triangles_to_render);

	printf("%d faces rendered\n", num_triangles);

	for (int i = 0; i < num_triangles; i++) {
		triangle_t face = triangles_to_render[i];

		// Draw vertex points
		draw_rect(face.points[0].x, face.points[0].y, 3, 3, 0xFFFFFF00);
		draw_rect(face.points[1].x, face.points[1].y, 3, 3, 0xFFFFFF00);
		draw_rect(face.points[2].x, face.points[2].y, 3, 3, 0xFFFFFF00);

		// Draw unfilled triangle
		draw_triangle(
				face.points[0].x, 
				face.points[0].y, 
				face.points[1].x, 
				face.points[1].y,
				face.points[2].x, 
				face.points[2].y,
				0xFF00FF00
		);
	}

	// Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);

	render_color_buffer();

	/* Vi rensker colorbufferen før hver frame rendres */
	clear_color_buffer(0xFF000000); // Svart

	SDL_RenderPresent(renderer);
}

////////////////////////////////////////////////////////////////////////////////
// Free memory that was dynamically allocated by the program
////////////////////////////////////////////////////////////////////////////////
void free_resources(void) {
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
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
	free_resources();
	
    return 0;
}
