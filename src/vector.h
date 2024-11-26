#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	float x;
	float y;
} vec2_t;

typedef struct {
	float x;
	float y;
	float z;
} vec3_t;

typedef struct {
	vec3_t position;
	vec3_t rotation;

} camera_t;

// TODO: Add functions to manipulate vectors 2D and 3D

#endif
