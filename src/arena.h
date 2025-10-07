typedef struct {
	void *base;		// Starten på minneområdet
	size_t size;	// Total størrelse på allokeringen
	size_t used;	// Hvor mye minne vi har brukt til nå
} arena_t;

void arena_init(arena_t* arena, size_t size);
void* arena_alloc(arena_t* arena, size_t size);
void arena_reset(arena_t* arena);
