#pragma once

#include "toy_common.h"

#include "literal.h"

typedef struct LiteralArray {
	Literal* literals;
	int capacity;
	int count;
} LiteralArray;

TOY_API void initLiteralArray(LiteralArray* array);
TOY_API void freeLiteralArray(LiteralArray* array);
TOY_API int pushLiteralArray(LiteralArray* array, Literal literal);
TOY_API Literal popLiteralArray(LiteralArray* array);
TOY_API bool setLiteralArray(LiteralArray* array, Literal index, Literal value);
TOY_API Literal getLiteralArray(LiteralArray* array, Literal index);

int findLiteralIndex(LiteralArray* array, Literal literal);
