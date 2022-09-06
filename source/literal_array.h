#pragma once

#include "literal.h"

typedef struct LiteralArray {
	Literal* literals;
	int capacity;
	int count;
} LiteralArray;

void initLiteralArray(LiteralArray* array);
void freeLiteralArray(LiteralArray* array);
int pushLiteralArray(LiteralArray* array, Literal literal);
Literal popLiteralArray(LiteralArray* array);
bool setLiteralArray(LiteralArray* array, Literal index, Literal value);
Literal getLiteralArray(LiteralArray* array, Literal index);

int findLiteralIndex(LiteralArray* array, Literal literal);
