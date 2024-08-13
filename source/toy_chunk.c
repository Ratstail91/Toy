#include "toy_chunk.h"

#include "toy_memory.h"

void Toy_initChunk(Toy_Chunk* chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
}

void Toy_pushChunk(Toy_Chunk* chunk, uint8_t byte) {
	if (chunk->count +1 > chunk->capacity) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = TOY_GROW_CAPACITY(oldCapacity);
		chunk->code = TOY_GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}

	chunk->code[chunk->count++] = byte;
}

void Toy_freeChunk(Toy_Chunk* chunk) {
	TOY_FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	Toy_initChunk(chunk);
}
