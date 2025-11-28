#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "mesh.h"
#include "array.h"

mesh_t mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = { 0, 0, 0 },
	.scale = { 1.0, 1.0, 1.0 },
	.translation = { 0, 0, 0 }
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
	{ .x = -1, .y = -1, .z = -1 }, // 1
	{ .x = -1, .y =  1, .z = -1 }, // 2
	{ .x =  1, .y =  1, .z = -1 }, // 3
	{ .x =  1, .y = -1, .z = -1 }, // 4
	{ .x =  1, .y =  1, .z =  1 }, // 5
	{ .x =  1, .y = -1, .z =  1 }, // 6
	{ .x = -1, .y =  1, .z =  1 }, // 7
	{ .x = -1, .y = -1, .z =  1 }, // 8
};

face_t cube_faces[N_CUBE_FACES] = {
	// front
	{ .a = 1, .b = 2, .c = 3, .color = 0xFFFF0000 },
	{ .a = 1, .b = 3, .c = 4, .color = 0xFFFF0000 },
	// right
	{ .a = 4, .b = 3, .c = 5, .color = 0xFF00FF00 },
	{ .a = 4, .b = 5, .c = 6, .color = 0xFF00FF00 },
	// back
	{ .a = 6, .b = 5, .c = 7, .color = 0xFF0000FF },
	{ .a = 6, .b = 7, .c = 8, .color = 0xFF0000FF },
	// left
	{ .a = 8, .b = 7, .c = 2, .color = 0xFFFFFF00 },
	{ .a = 8, .b = 2, .c = 1, .color = 0xFFFFFF00 },
	// top
	{ .a = 2, .b = 7, .c = 5, .color = 0xFFFF00FF },
	{ .a = 2, .b = 5, .c = 3, .color = 0xFFFF00FF },
	// bottom
	{ .a = 6, .b = 8, .c = 1, .color = 0xFF00FFFF },
	{ .a = 6, .b = 1, .c = 4, .color = 0xFF00FFFF }
};

void load_cube_mesh_data(void) {
	for (int i = 0; i < N_CUBE_VERTICES; i++) {
		vec3_t cube_vertex = cube_vertices[i];
		array_push(mesh.vertices, cube_vertex);
	}
	for (int i = 0; i < N_CUBE_FACES; i++) {
		face_t cube_face = cube_faces[i];
		array_push(mesh.faces, cube_face);
	}
}

void load_obj_file_data(char* filename) {
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		printf("Failed to open file\n");
		exit(1);
	}
	char* buf = malloc(8192);

	// Reading text file, line by line
	while (feof(f) == 0) {
		fgets(buf, 80, f);

		// Reading and pushing vector onto global mesh
		if ((char)buf[0] == 'v' && (char)buf[1] == ' ') {
			char* vector_buf = malloc(80); // stores the char buffer for a float
			int v_buf_index = 0;

			vec3_t vertex = { .x = 0, .y = 0, .z = 0 };
			int vertex_index = 0;

			for (int i = 2; i < 80; i++) {
				if (buf[i] == ' ' || buf[i] == '\0') { // store current vector in vertex
					float v = atof(vector_buf);
					if (vertex_index == 0) {
						vertex.x = v;
					} else if (vertex_index == 1) {
						vertex.y = v;
					} else if (vertex_index == 2) {
						vertex.z = v;
					}
					vertex_index++;
					free(vector_buf);
					vector_buf = malloc(80);
					v_buf_index = 0;
					if (buf[i] == '\0') break; // reached EOL, reading newline
					continue;
				}
				// Write char from buffer into vector_buffer
				vector_buf[v_buf_index] = buf[i];
				v_buf_index++;
			}

			free(vector_buf);
			// We push the current vector onto the mesh
			array_push(mesh.vertices, vertex);
		}

		// Reading and pushing face onto global mesh
		if (buf[0] == 'f') {
			char* index_buf = malloc(80);
			int i_buf_index = 0;

			face_t face = { .a = 0, .b = 0, .c = 0 };
			int face_index = 0;
			int i = 2;
			while (i < 80) {
				if (buf[i] == '/') { // we skip the texture and normal indices
					while (buf[i] != ' ') {
						i++;
					}
				}
				if (buf[i] == '\0') {
					break;
				}
				if (buf[i] == ' ') { // store current index in face (vertex index)
					int index = atoi(index_buf);
					if (face_index == 0) {
						face.a = index;
					} else if (face_index == 1) {
						face.b = index;
					} else {
						face.c = index;
					}
					face_index++;
					free(index_buf);
					index_buf = malloc(80);
					i_buf_index = 0;
					i++;
					continue;
				}

				// Write char from buffer to the face index buffer
				index_buf[i_buf_index] = buf[i];
				i_buf_index++;
				i++;
			}

			// Push the current face onto the mesh
			array_push(mesh.faces, face);
		}
	}
	// Freeing up used resources
	fclose(f);
	free(buf);
}

