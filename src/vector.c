#include <math.h>
#include "vector.h"

// TODO: Implementation of all vector functions

////////////////////////////////////////////////////////////////////////////////
/// Funksjon som mottar en 3D vektor og returnerer et projektert 2D punkt
/// Vi trenger å bruke projeksjon for å vise et 3D objekt på en 2D skjermflate
/// Orthographisk projeksjon
////////////////////////////////////////////////////////////////////////////////
vec2_t project(vec3_t v, float fov_factor) {
	vec2_t projected_point = {
		.x = (fov_factor * v.x) / v.z, // Skalerer og legger på dybde effekt
		.y = (fov_factor * v.y) / v.z
	};
	return projected_point;
}

////////////////////////////////////////////////////////////////////////////////
/// Funksjon som roterer en 3D vektor rundt x-aksen
////////////////////////////////////////////////////////////////////////////////
vec3_t vec3_rotate_x(vec3_t v, float angle) {
	vec3_t rotated_vector = {
		.x = v.x, // Holdes konstant
		.y = v.y * cos(angle) - v.z * sin(angle),
		.z = v.y * sin(angle) + v.z * cos(angle)
	};
	return rotated_vector;
}

////////////////////////////////////////////////////////////////////////////////
/// Funksjon som roterer en 3D vektor rundt z-aksen
////////////////////////////////////////////////////////////////////////////////
vec3_t vec3_rotate_z(vec3_t v, float angle) {
	vec3_t rotated_vector = {
		.x = v.x * cos(angle) - v.y * sin(angle),
		.y = v.x * sin(angle) + v.y * cos(angle),
		.z = v.z // Holdes konstant

	};
	return rotated_vector;
}

////////////////////////////////////////////////////////////////////////////////
/// Funksjon som roterer en 3D vektor rundt y-aksen
////////////////////////////////////////////////////////////////////////////////
vec3_t vec3_rotate_y(vec3_t v, float angle) {
	vec3_t rotated_vector = {
		.x = v.x * cos(angle) - v.z * sin(angle),
		.y = v.y, // Holdes konstant
		.z = v.x * sin(angle) + v.z * cos(angle),

	};
	return rotated_vector;
}
