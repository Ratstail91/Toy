#pragma once

#include "toy_common.h"

#include "literal.h"

//TODO: benchmark this
#define DICTIONARY_MAX_LOAD 0.75

typedef struct _entry {
	Literal key;
	Literal value;
} _entry;

typedef struct LiteralDictionary {
	_entry* entries;
	int capacity;
	int count;
	int contains; //count + tombstones, for internal use
} LiteralDictionary;

TOY_API void initLiteralDictionary(LiteralDictionary* dictionary);
TOY_API void freeLiteralDictionary(LiteralDictionary* dictionary);

TOY_API void setLiteralDictionary(LiteralDictionary* dictionary, Literal key, Literal value);
TOY_API Literal getLiteralDictionary(LiteralDictionary* dictionary, Literal key);
TOY_API void removeLiteralDictionary(LiteralDictionary* dictionary, Literal key);

TOY_API bool existsLiteralDictionary(LiteralDictionary* dictionary, Literal key);
