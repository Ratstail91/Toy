#pragma once

#include "literal.h"

typedef struct LiteralArray {
	int capacity;
	int count;
	Literal* literals;
} LiteralArray;

void initLiteralArray(LiteralArray* array);
int pushLiteralArray(LiteralArray* array, Literal literal);
Literal popLiteralArray(LiteralArray* array);
void freeLiteralArray(LiteralArray* array);

int findLiteralIndex(LiteralArray* array, Literal literal);

void printLiteralArray(LiteralArray* array, const char* delim);