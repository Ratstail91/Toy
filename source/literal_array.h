#pragma once

#include "literal.h"

typedef struct LiteralArray {
	int capacity;
	int count;
	Literal* literals;
} LiteralArray;

void initLiteralArray(LiteralArray* array);
int writeLiteralArray(LiteralArray* array, Literal literal);
void freeLiteralArray(LiteralArray* array);

int findLiteralIndex(LiteralArray* array, Literal literal);

