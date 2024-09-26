#include "toy_stack.h"
#include "toy_console_colors.h"

#include "toy_memory.h"

#include <stdio.h>
#include <stdlib.h>

//a good chunk of space
#define MIN_SIZE 64

TOY_API void Toy_initStack(Toy_Stack* stack) {
	stack->ptr = NULL;
	stack->capacity = 0;
	stack->count = 0;
}

void Toy_preallocateStack(Toy_Stack* stack) {
	stack->capacity = MIN_SIZE;
	stack->count = 0;

	stack->ptr = TOY_ALLOCATE(Toy_Value, stack->capacity);
}

void Toy_freeStack(Toy_Stack* stack) {
	//TODO: slip in a call to free the complex values here

	TOY_FREE_ARRAY(Toy_Value, stack->ptr, stack->capacity);

	Toy_initStack(stack);
}

void Toy_pushStack(Toy_Stack* stack, Toy_Value value) {
	//don't go overboard - limit to 1mb
	if (stack->count >= 1024 * 1024 / sizeof(Toy_Value)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack overflow, exiting\n" TOY_CC_RESET);
		exit(-1);
	}

	//expand the capacity if needed
	if (stack->count + 1 > stack->capacity) {
		int oldCapacity = stack->capacity;
		stack->capacity = TOY_GROW_CAPACITY(stack->capacity);
		stack->ptr = TOY_GROW_ARRAY(Toy_Value, stack->ptr, oldCapacity, stack->capacity);
	}

	stack->ptr[stack->count++] = value;
}

Toy_Value Toy_peekStack(Toy_Stack* stack) {
	if (stack->count <= 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow, exiting\n" TOY_CC_RESET);
		exit(-1);
	}

	return stack->ptr[stack->count - 1];
}

Toy_Value Toy_popStack(Toy_Stack* stack) {
	if (stack->count <= 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow, exiting\n" TOY_CC_RESET);
		exit(-1);
	}

	//shrink if possible
	if (stack->count > MIN_SIZE && stack->count < stack->capacity / 4) {
		stack->ptr = TOY_SHRINK_ARRAY(Toy_Value, stack->ptr, stack->capacity, stack->capacity / 2);
		stack->capacity /= 2;
	}

	return stack->ptr[--stack->count];
}
