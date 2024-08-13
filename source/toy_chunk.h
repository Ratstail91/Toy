#pragma once

#include "toy_common.h"

typedef struct Toy_Chunk {
	int count;
	int capacity;
	uint8_t* code;
} Toy_Chunk;

TOY_API void Toy_initChunk(Toy_Chunk* chunk);
TOY_API void Toy_pushChunk(Toy_Chunk* chunk, uint8_t byte);
TOY_API void Toy_freeChunk(Toy_Chunk* chunk);
