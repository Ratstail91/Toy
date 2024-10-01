#pragma once

#include "toy_common.h"

//standard generic array
typedef struct Toy_Array { //32 | 64 BITNESS
	size_t capacity;       //4  | 4
	size_t count;          //4  | 4
	char data[];           //-  | -
} Toy_Array;               //8  | 8

TOY_API Toy_Array* Toy_resizeArray(Toy_Array* array, size_t capacity);

#define TOY_ALLOCATE_ARRAY(type, count) \
	Toy_resizeArray(NULL, sizeof(type)*(count))

#define TOY_FREE_ARRAY(type, array) \
	Toy_resizeArray(array, 0)

#define TOY_ADJUST_ARRAY(type, array, newCapacity) \
	Toy_resizeArray(array, sizeof(type) * newCapacity)

#define TOY_DOUBLE_ARRAY_CAPACITY(type, array) \
	Toy_resizeArray(array, sizeof(type) * array->capacity < 8 ? sizeof(type) * 8 : sizeof(type) * array->capacity * 2)

