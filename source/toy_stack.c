#include "toy_stack.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//a good chunk of space - 'count' actually tracks the number of values
#define MIN_CAPACITY 64

Toy_Stack* Toy_allocateStack() {
	Toy_Stack* stack = malloc(MIN_CAPACITY * sizeof(Toy_Value) + sizeof(Toy_Stack));

	if (stack == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, MIN_CAPACITY, (int)(MIN_CAPACITY * sizeof(Toy_Value) + sizeof(Toy_Stack)));
		exit(1);
	}

	stack->capacity = MIN_CAPACITY;
	stack->count = 0;

	return stack;
}

void Toy_freeStack(Toy_Stack* stack) {
	//TODO: slip in a call to free the complex values here

	free(stack);
}

void Toy_pushStack(Toy_Stack** stack, Toy_Value value) {
	//don't go overboard - limit to 1mb of capacity used
	if ((*stack)->count >= 1024 * 1024 / sizeof(Toy_Value)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack overflow\n" TOY_CC_RESET);
		exit(-1);
	}

	//expand the capacity if needed
	if ((*stack)->count + 1 > (*stack)->capacity) {
		while ((*stack)->count + 1 > (*stack)->capacity) {
			(*stack)->capacity = (*stack)->capacity < MIN_CAPACITY ? MIN_CAPACITY : (*stack)->capacity * 2;
		}

		size_t newCapacity = (*stack)->capacity;

		(*stack) = realloc((*stack), newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack));

		if ((*stack) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reallocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, (int)newCapacity, (int)(newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack)));
			exit(1);
		}
	}

	//Note: "pointer arithmetic in C/C++ is type-relative"
	((Toy_Value*)((*stack) + 1))[(*stack)->count++] = value;
}

Toy_Value Toy_peekStack(Toy_Stack** stack) {
	if ((*stack)->count == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow\n" TOY_CC_RESET);
		exit(-1);
	}

	return ((Toy_Value*)((*stack) + 1))[(*stack)->count - 1];
}

Toy_Value Toy_popStack(Toy_Stack** stack) {
	if ((*stack)->count == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow\n" TOY_CC_RESET);
		exit(-1);
	}

	//shrink if possible
	if ((*stack)->count > MIN_CAPACITY && (*stack)->count < (*stack)->capacity / 4) {
		(*stack)->capacity /= 2;
		size_t newCapacity = (*stack)->capacity;

		(*stack) = realloc((*stack), (*stack)->capacity * sizeof(Toy_Value) + sizeof(Toy_Stack));

		if ((*stack) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reallocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, (int)newCapacity, (int)(newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack)));
			exit(1);
		}
	}

	return ((Toy_Value*)((*stack) + 1))[--(*stack)->count];
}
