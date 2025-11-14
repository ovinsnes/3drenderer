#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vector.h"

////////////////////////////////////////////////////////////////////////////////
/// Stores the index of each vertex in a face
////////////////////////////////////////////////////////////////////////////////
typedef struct {
	int a;
	int b;
	int c;
	uint32_t color;
} face_t;

////////////////////////////////////////////////////////////////////////////////
/// Stores the actual projected points of the triangle in the screen
////////////////////////////////////////////////////////////////////////////////
typedef struct {
	vec2_t points[3]; // Offsets 0,4,8,12,16,20
					  // Size: 24 bytes (3x8 bytes)
	uint32_t color;   // Offset 24: uint32_t color (4 bytes)
} triangle_t;		  // Total size: 28 bytes

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

#endif
