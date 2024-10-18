#pragma once

#include "toy_common.h"
#include "toy_value.h"

//standard generic array
typedef struct Toy_Array { //32 | 64 BITNESS
	unsigned int capacity; //4  | 4
	unsigned int count;    //4  | 4
	Toy_Value data[];      //-  | -
} Toy_Array;               //8  | 8

TOY_API Toy_Array* Toy_resizeArray(Toy_Array* array, unsigned int capacity);
