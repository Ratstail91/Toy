#pragma once

#include "literal.h"

typedef struct LiteralArray {
	Literal* literals;
	int capacity;
	int count;
} LiteralArray;

void initLiteralArray(LiteralArray* array);
int pushLiteralArray(LiteralArray* array, Literal literal);
Literal popLiteralArray(LiteralArray* array);
void freeLiteralArray(LiteralArray* array);

int findLiteralIndex(LiteralArray* array, Literal literal);
