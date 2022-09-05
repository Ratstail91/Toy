#pragma once

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

void initLiteralDictionary(LiteralDictionary* dictionary);
void freeLiteralDictionary(LiteralDictionary* dictionary);

void setLiteralDictionary(LiteralDictionary* dictionary, Literal key, Literal value);
Literal getLiteralDictionary(LiteralDictionary* dictionary, Literal key);
void removeLiteralDictionary(LiteralDictionary* dictionary, Literal key);

bool existsLiteralDictionary(LiteralDictionary* dictionary, Literal key);
