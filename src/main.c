#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <time.h>

#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "arena.h"
#include "matrix.h"
	
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
#include <immintrin.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Pre-allokert håndtering av minne
////////////////////////////////////////////////////////////////////////////////
triangle_t* triangles_to_render = NULL;

arena_t global_arena;
int max_triangles = 0;
int triangle_count = 0;

////////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
////////////////////////////////////////////////////////////////////////////////
TTF_Font* font = NULL;
int frame_count = 0;
int fps_timer = 0;
int current_fps = 0;
bool show_rendermode_message = false;
int rendermode_message_timer = 0;

SDL_Color white = {255, 255, 255};
SDL_Color green = {0, 255, 0};

bool is_running = false;
int previous_frame_time = 0;

bool draw_wireframe = true;
bool draw_filled_polygons = false;
bool draw_filled_wireframe = false;
char key_pressed;

enum render_modes {
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE
} render_modes;

bool apply_culling = true;
bool matrix_transforms = false;
bool simd = false;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
float fov_factor = 640;
mat4_t proj_matrix;

////////////////////////////////////////////////////////////////////////////////
// Setup function to initialize variables and game objects
////////////////////////////////////////////////////////////////////////////////
void setup(void) {
	// Alloker nødvendig antall bytes i minnet for fargebuffer
	arena_init(&global_arena, 64 * 1024 * 1024);	// 64MB
	color_buffer = (uint32_t*) arena_alloc(&global_arena, sizeof(uint32_t) * window_width * window_height);
	render_modes = RENDER_WIRE;

	// Lage en SDL texture for å vise fargebufferet
	color_buffer_texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			window_width,
			window_height
	);

	// Initialiser projeksjonsmatrisen
	float fov = M_PI / 3.0; // tilsvarende 180/3, eller 60 grader
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

	// Loads the cube values in the mesh data structure
	clock_t start, end;
	double cpu_time_used;

	start = clock();
	load_obj_file_data("./assets/bunny.obj");
	//load_cube_mesh_data();
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("load_obj_data() took %f milliseconds to execute\n", cpu_time_used * 1000.0);

	// Pre-alloker minne for hele trådmodellen
	max_triangles = array_length(mesh.faces);
	triangles_to_render = (triangle_t*) arena_alloc(&global_arena, max_triangles * sizeof(triangle_t));

	// Initialize SDL_ttf
	if (TTF_Init() != 0) {
	    fprintf(stderr, "Error initializing SDL_ttf: %s\n", TTF_GetError());
	}
	font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 24);
	if (font == NULL) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
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
			if (event.key.keysym.sym == SDLK_1) {
                render_modes = RENDER_WIRE_VERTEX;
				key_pressed = 1;
                show_rendermode_message = true;
                rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_2) {
				render_modes = RENDER_WIRE;
				key_pressed = 2;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_3) {
				render_modes = RENDER_FILL_TRIANGLE;
				key_pressed = 3;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_4) {
				render_modes = RENDER_FILL_TRIANGLE_WIRE;
				key_pressed = 4;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_5) {
				apply_culling = !apply_culling;
				key_pressed = 5;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_6) {
				matrix_transforms = !matrix_transforms;
				key_pressed = 6;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			if (event.key.keysym.sym == SDLK_7) {
				simd = !simd;
				key_pressed = 7;
				show_rendermode_message = true;
				rendermode_message_timer = SDL_GetTicks();
			}
			break;
	}
}



void update(void) {
	// We "lock" execution until FRAME_TARGET_TIME has passed to create a
	// consistent frame time
//	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
//
//	// Only delay execution if we are running too fast
//	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
//		SDL_Delay(time_to_wait);
//	}

	previous_frame_time = SDL_GetTicks(); // Number of ms since game has started

	// FPS kalkulering
	frame_count++;
	if (SDL_GetTicks() - fps_timer >= 1000) { // Vi har passert 1 sekund
	    current_fps = frame_count;
        frame_count = 0;
        fps_timer = SDL_GetTicks();
    }
	
	triangle_count = 0;	// Reset counter

	// Endre rotasjon/skalering per frame
	mesh.rotation.x += 0.01; // Speed of rotation
	mesh.rotation.y += 0.01; // Speed of rotation
	mesh.rotation.z += 0.01; // Speed of rotation
//	mesh.scale.x += 0.002;
//	mesh.scale.y += 0.001;
	// Hold objektet på fast posisjon foran kameraet
	mesh.translation.z = 5.0; // Sett objektet 5 enheter foran kameraet

	// Lage en skalering, rotasjon, og translasjonsmatrise for å multiplisere punktene i meshet
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	// Verdensmatrise som kombinerer skalering, rotasjon og translasjon
	mat4_t world_matrix = mat4_identity();
	// Appliser transformasjoner i riktig rekkefølge: Scale -> Rotate -> Translate
	world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
	world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
	world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

	// Loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);

	for (int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];

		// Array starts at index 0 so we compensate by subtracting 1
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec3_t transformed_vertices[3];
		vec4_t transformed_vec4s[3];

		// Loop all 3 vertices of current face and apply *transformations*
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];
			vec4_t transformed_vec4;

			if (matrix_transforms) {
				transformed_vec4 = vec4_from_vec3(face_vertices[j]);
				// Multipliser verdensmatrisen med den opprinnelige vektoren
				if (simd) {
					transformed_vec4 = mat4_mul_vec4_simd(world_matrix, transformed_vec4);
				} else {
					transformed_vec4 = mat4_mul_vec4(world_matrix, transformed_vec4);
				}
				transformed_vec4s[j] = transformed_vec4;

				// Populer også transformed_vertices for avg_depth beregning
				transformed_vertices[j] = vec3_from_vec4(transformed_vec4);
			} else {
				// Rotate the points 0.1 degrees around y-axis for every frame update
				transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
				transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
				transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

				// Translate the vertex away from the camera in the z-plane
				transformed_vertex.z += 5;
				// Lagre punktene i array for transformerte punkter
				transformed_vertices[j] = transformed_vertex;
			}
		}


		////////////////////////////////////////////////////////////////////////////////
		/// Backface Culling
		///////////////////////////////////////////////////////////////////////////////
		if (apply_culling) {
			vec3_t vector_a;
			vec3_t vector_b;
			vec3_t vector_c;

			if (matrix_transforms) {
				vector_a = vec3_from_vec4(transformed_vec4s[0]);
				vector_b = vec3_from_vec4(transformed_vec4s[1]);
				vector_c = vec3_from_vec4(transformed_vec4s[2]);
			} else {
				vector_a = transformed_vertices[0];
				vector_b = transformed_vertices[1];
				vector_c = transformed_vertices[2];
			}
			vec3_t vector_ab = vec3_sub(vector_a, vector_b);
			vec3_t vector_ac = vec3_sub(vector_a, vector_c);

			vec3_normalize(&vector_ab);
			vec3_normalize(&vector_ac);

			// Finn kryssproduktet mellom dem (venstre-hånd koordinatsystem)
			vec3_t normal = vec3_cross(vector_ab, vector_ac);
			// Normaliser normalen for planet
			vec3_normalize(&normal);

			// Finn kamera strålevektoren
			vec3_t camera_ray = vec3_sub(camera_position, vector_a);

			// Punktproduktet mellom kamerastrålen og normalen N
			float dot = vec3_dot(normal, camera_ray);

			// Sjekk om flaten skal vises eller gjemmes bort
			if (dot <= 0) {
				continue;
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		/// Projection
		///////////////////////////////////////////////////////////////////////////////
		vec2_t projected_points[3];
		vec4_t mat_proj_points[3];

		
		// Loop gjennom alle 3 punktene av den nåværende flaten og *projiser* dem
		for (int j = 0; j < 3; j++) {
			// Project the current vertex onto 2D space
			if (matrix_transforms) {
				mat_proj_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vec4s[j]);
				// Skaler opp i viewport
				mat_proj_points[j].x *= (window_width / 4.0);
				mat_proj_points[j].y *= (window_height / 4.0);
				// Translate the projected points to the middle of the screen
				mat_proj_points[j].x += (window_width / 2.0);
				mat_proj_points[j].y += (window_height / 2.0);

			} else {
				projected_points[j] = project(transformed_vertices[j], fov_factor);
				// Scale and translate the projected points to the middle of the screen
				projected_points[j].x += (window_width / 2);
				projected_points[j].y += (window_height / 2);
			}
		}

		// Average z-value after transformation
		float avg_depth = (
				transformed_vertices[0].z +
				transformed_vertices[1].z +
				transformed_vertices[2].z
				) / 3.0;

		triangle_t projected_triangle;

		if (matrix_transforms) {
			projected_triangle.points[0].x = mat_proj_points[0].x;
			projected_triangle.points[0].y = mat_proj_points[0].y;
			projected_triangle.points[1].x = mat_proj_points[1].x;
			projected_triangle.points[1].y = mat_proj_points[1].y;
			projected_triangle.points[2].x = mat_proj_points[2].x;
			projected_triangle.points[2].y = mat_proj_points[2].y;
			projected_triangle.color = mesh_face.color;
			projected_triangle.avg_depth = avg_depth;
		} else {
			projected_triangle.points[0].x = projected_points[0].x;
			projected_triangle.points[0].y = projected_points[0].y;
			projected_triangle.points[1].x = projected_points[1].x;
			projected_triangle.points[1].y = projected_points[1].y;
			projected_triangle.points[2].x = projected_points[2].x;
			projected_triangle.points[2].y = projected_points[2].y;
			projected_triangle.color = mesh_face.color;
			projected_triangle.avg_depth = avg_depth;
		}

		// Save the transformed and projected triangle in the array of triangles to render
		//array_push(triangles_to_render, projected_triangle);
		triangles_to_render[triangle_count++] = projected_triangle;
	}

	// TODO OLE: Sort the triangles to render by their average z-value
	// (average depth) after pushing them to memory buffer
	bool pass = false;
	while (pass) {
		pass = false;
		for (int i = 0; i < triangle_count; i++) {
			if (triangles_to_render[i].avg_depth > triangles_to_render[i+1].avg_depth) {
				triangle_t tmp = triangles_to_render[i+1];
				triangles_to_render[i+1] = triangles_to_render[i];
				triangles_to_render[i] = tmp;
				pass = true;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
////////////////////////////////////////////////////////////////////////////////
void render(void) {
	draw_grid();

	// Loop gjennom og rendre alle projekterte triangler (faces)
	int num_triangles = triangle_count;

	//printf("%d faces rendered\n", num_triangles);

	for (int i = 0; i < num_triangles; i++) {
		triangle_t face = triangles_to_render[i];


		if (render_modes == RENDER_FILL_TRIANGLE || render_modes == RENDER_FILL_TRIANGLE_WIRE) {
			// Draw filled triangle
			draw_filled_triangle(
					face.points[0].x, 
					face.points[0].y, 
					face.points[1].x, 
					face.points[1].y,
					face.points[2].x, 
					face.points[2].y,
					face.color
			);
		}

		if (render_modes == RENDER_WIRE || render_modes == RENDER_WIRE_VERTEX || render_modes == RENDER_FILL_TRIANGLE_WIRE) {
			// Draw unfilled triangle
			draw_triangle(
					face.points[0].x, 
					face.points[0].y, 
					face.points[1].x, 
					face.points[1].y,
					face.points[2].x, 
					face.points[2].y,
					0xFFFFFFFF
			);
		}

		if (render_modes == RENDER_WIRE_VERTEX) {
			// Draw vertex points
			draw_rect(face.points[0].x, face.points[0].y, 3, 3, 0xFFFF0000);
			draw_rect(face.points[1].x, face.points[1].y, 3, 3, 0xFFFF0000);
			draw_rect(face.points[2].x, face.points[2].y, 3, 3, 0xFFFF0000);
		}
	}

	render_color_buffer();
	/* Vi rensker colorbufferen før hver frame rendres */
	clear_color_buffer(0xFF000000); // Svart

	// Render FPS counter
	char fps_text[10];
	sprintf(fps_text, "FPS: %d", current_fps);
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, fps_text, white);
    if (text_surface) {
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        if (text_texture) {
            SDL_Rect text_rect = {10, 10, text_surface->w, text_surface->h};
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
        SDL_FreeSurface(text_surface);
    }

	// Render face counter (antal trekanter som rendres)
	char face_count_text[50];
	sprintf(face_count_text, "Faces: %d / %d", num_triangles, max_triangles);
	SDL_Surface* face_surface = TTF_RenderText_Solid(font, face_count_text, white);
    if (face_surface) {
        SDL_Texture* face_texture = SDL_CreateTextureFromSurface(renderer, face_surface);
        if (face_texture) {
            SDL_Rect face_rect = {10, 40, face_surface->w, face_surface->h};
            SDL_RenderCopy(renderer, face_texture, NULL, &face_rect);
            SDL_DestroyTexture(face_texture);
        }
        SDL_FreeSurface(face_surface);
    }

	// Vise infotekst om render modus på bunnen
	char render_info_text[300];
	sprintf(
			render_info_text, 
			"Press '1' for wireframe and vertex toggle || "
			"Press '2' for wireframe only || "
			"Press '3' for filled triangles || " 
			"Press '4' for filled triangles with wireframe || "
			"Press '5' for backface culling toggle || "
			"Press '6' for matrix transformations toggle"
			);
	SDL_Surface* info_surface = TTF_RenderText_Solid(font, render_info_text, green);
    if (info_surface) {
        SDL_Texture* info_texture = SDL_CreateTextureFromSurface(renderer, info_surface);
        if (info_surface) {
            SDL_Rect text_rect = {10, window_height - 100, info_surface->w, info_surface->h};
            SDL_RenderCopy(renderer, info_texture, NULL, &text_rect);
            SDL_DestroyTexture(info_texture);
        }
        SDL_FreeSurface(info_surface);
    }

    // Render wireframe status melding (hvs aktiv)
    if (show_rendermode_message && font) {
		// Sjekk om 3 sekunder har gått
		if (SDL_GetTicks() - rendermode_message_timer >= 3000) {
			show_rendermode_message = false;
		} else {
			char rendermode_text[50];

			switch (key_pressed) {
				case 1:
					sprintf(rendermode_text, "Rendering wireframe and vertices");
					break;
				case 2:
					sprintf(rendermode_text, "Rendering wireframe only");
					break;
				case 3:
					sprintf(rendermode_text, "Rendering filled triangles");
					break;
				case 4:
					sprintf(rendermode_text, "Rendering filled triangles with wireframe");
					break;
				case 5:
					sprintf(rendermode_text, "Backface culling toggled %s", apply_culling ? "ON" : "OFF");
					break;
				case 6:
					sprintf(rendermode_text, "Matrix transformations toggled %s", matrix_transforms ? "ON" : "OFF");
					break;
				case 7:
					sprintf(rendermode_text, "SIMD toggled %s", simd ? "ON" : "OFF");
					break;


			}
			SDL_Surface* wireframe_surface = TTF_RenderText_Solid(font, rendermode_text, white);
			if (wireframe_surface) {
				SDL_Texture* wireframe_texture = SDL_CreateTextureFromSurface(renderer, wireframe_surface);
				if (wireframe_texture) {
					SDL_Rect wireframe_rect = {10, 80, wireframe_surface->w, wireframe_surface->h};
					SDL_RenderCopy(renderer, wireframe_texture, NULL, &wireframe_rect);
					SDL_DestroyTexture(wireframe_texture);
				}
				SDL_FreeSurface(wireframe_surface);
			}

		}
    }

	SDL_RenderPresent(renderer);
}

////////////////////////////////////////////////////////////////////////////////
// Free memory that was dynamically allocated by the program
////////////////////////////////////////////////////////////////////////////////
void free_resources(void) {
if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(void) {
	is_running = initialize_window();

	setup();

	// Initialize the timer used for FPS calculations
    fps_timer = SDL_GetTicks();

	// Game loop

	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();
	free_resources();
	
    return 0;
}
