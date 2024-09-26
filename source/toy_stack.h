#pragma once

#include "toy_common.h"
#include "toy_value.h"

typedef struct Toy_Stack {
	Toy_Value* ptr;
	int capacity;
	int count;
} Toy_Stack;

TOY_API void Toy_initStack(Toy_Stack* stack); //null memory
TOY_API void Toy_preallocateStack(Toy_Stack* stack); //non-null memory, ready to go
TOY_API void Toy_freeStack(Toy_Stack* stack);

TOY_API void Toy_pushStack(Toy_Stack* stack, Toy_Value value);
TOY_API Toy_Value Toy_peekStack(Toy_Stack* stack);
TOY_API Toy_Value Toy_popStack(Toy_Stack* stack);
