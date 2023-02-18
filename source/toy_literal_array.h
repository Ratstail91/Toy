#pragma once

#include "toy_common.h"

#include "toy_literal.h"

typedef struct Toy_LiteralArray {
	Toy_Literal* literals;
	int capacity;
	int count;
} Toy_LiteralArray;

TOY_API void Toy_initLiteralArray(Toy_LiteralArray* array);
TOY_API void Toy_freeLiteralArray(Toy_LiteralArray* array);
TOY_API int Toy_pushLiteralArray(Toy_LiteralArray* array, Toy_Literal literal);
TOY_API Toy_Literal Toy_popLiteralArray(Toy_LiteralArray* array);
TOY_API bool Toy_setLiteralArray(Toy_LiteralArray* array, Toy_Literal index, Toy_Literal value);
TOY_API Toy_Literal Toy_getLiteralArray(Toy_LiteralArray* array, Toy_Literal index);

int Toy_findLiteralIndex(Toy_LiteralArray* array, Toy_Literal literal);

//TODO: add a function to get the capacity & count