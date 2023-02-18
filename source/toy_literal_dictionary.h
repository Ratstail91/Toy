#pragma once

#include "toy_common.h"

#include "toy_literal.h"

//TODO: benchmark this
#define TOY_DICTIONARY_MAX_LOAD 0.75

typedef struct Toy_private_dictionary_entry {
	Toy_Literal key;
	Toy_Literal value;
} Toy_private_dictionary_entry;

typedef struct Toy_LiteralDictionary {
	Toy_private_dictionary_entry* entries;
	int capacity;
	int count;
	int contains; //count + tombstones, for internal use
} Toy_LiteralDictionary;

TOY_API void Toy_initLiteralDictionary(Toy_LiteralDictionary* dictionary);
TOY_API void Toy_freeLiteralDictionary(Toy_LiteralDictionary* dictionary);

TOY_API void Toy_setLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key, Toy_Literal value);
TOY_API Toy_Literal Toy_getLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key);
TOY_API void Toy_removeLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key);

TOY_API bool Toy_existsLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key);
