#include <stdlib.h>

#include "arena.h"

void arena_init(arena_t* arena, size_t size) {
	arena->base = malloc(size);		// Én stor malloc
	arena->size = size;
	arena->used = 0;
}

void* arena_alloc(arena_t* arena, size_t size) {
	size_t aligned_size = (size + 7) & ~7; // Vi passer på at alle allokerte
										   // objekter blir plassert i
										   // separate chunks på 8 byte. Dette
										   // gjør at allokeringer ikke spres på
										   // tvers av cache linjen (64b).
										   // Objekter mindre enn 8 byte blir
										   // paddet med "tomme bytes".
	
	size_t aligned_start = (arena->used + 7) & ~7;	// Vi gjør det samme med
													// start-posisjonen for den
													// nye blokken også

	if (arena->size < (arena->used + aligned_size)) { // Sjekk om vi har nok
													 // plass igjen
		return NULL;
	}
	void* current_pos = (char*) arena->base + aligned_start;
	arena->used += aligned_size;
	return current_pos;
}
