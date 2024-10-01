#pragma once

#include "toy_common.h"
#include "toy_value.h"

typedef struct Toy_Stack { //32 | 64 BITNESS
	size_t capacity;       //4  | 4
	size_t count;          //4  | 4
	char data[];           //-  | -
} Toy_Stack;               //8  | 8

TOY_API Toy_Stack* Toy_allocateStack();
TOY_API void Toy_freeStack(Toy_Stack* stack);

TOY_API void Toy_pushStack(Toy_Stack** stack, Toy_Value value);
TOY_API Toy_Value Toy_peekStack(Toy_Stack** stack);
TOY_API Toy_Value Toy_popStack(Toy_Stack** stack);
