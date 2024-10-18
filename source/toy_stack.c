#include "toy_stack.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

Toy_Stack* Toy_allocateStack() {
	Toy_Stack* stack = malloc(TOY_STACK_INITIAL_CAPACITY * sizeof(Toy_Value) + sizeof(Toy_Stack));

	if (stack == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, TOY_STACK_INITIAL_CAPACITY, (int)(TOY_STACK_INITIAL_CAPACITY * sizeof(Toy_Value) + sizeof(Toy_Stack)));
		exit(1);
	}

	stack->capacity = TOY_STACK_INITIAL_CAPACITY;
	stack->count = 0;

	return stack;
}

void Toy_freeStack(Toy_Stack* stack) {
	//TODO: slip in a call to free the complex values here

	if (stack != NULL) {
		free(stack);
	}
}

void Toy_pushStack(Toy_Stack** stackHandle, Toy_Value value) {
	//don't go overboard
	if ((*stackHandle)->count >= TOY_STACK_OVERFLOW) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack overflow\n" TOY_CC_RESET);
		exit(-1);
	}

	//expand the capacity if needed
	if ((*stackHandle)->count + 1 > (*stackHandle)->capacity) {
		while ((*stackHandle)->count + 1 > (*stackHandle)->capacity) {
			(*stackHandle)->capacity = (*stackHandle)->capacity < TOY_STACK_INITIAL_CAPACITY ? TOY_STACK_INITIAL_CAPACITY : (*stackHandle)->capacity * TOY_STACK_EXPANSION_RATE;
		}

		unsigned int newCapacity = (*stackHandle)->capacity;

		(*stackHandle) = realloc((*stackHandle), newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack));

		if ((*stackHandle) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reallocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, (int)newCapacity, (int)(newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack)));
			exit(1);
		}
	}

	//Note: "pointer arithmetic in C/C++ is type-relative"
	((Toy_Value*)((*stackHandle) + 1))[(*stackHandle)->count++] = value;
}

Toy_Value Toy_peekStack(Toy_Stack** stackHandle) {
	if ((*stackHandle)->count == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow\n" TOY_CC_RESET);
		exit(-1);
	}

	return ((Toy_Value*)((*stackHandle) + 1))[(*stackHandle)->count - 1];
}

Toy_Value Toy_popStack(Toy_Stack** stackHandle) {
	if ((*stackHandle)->count == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Stack underflow\n" TOY_CC_RESET);
		exit(-1);
	}

	//shrink if possible
	if ((*stackHandle)->count > TOY_STACK_INITIAL_CAPACITY && (*stackHandle)->count < (*stackHandle)->capacity * TOY_STACK_CONTRACTION_RATE) {
		(*stackHandle)->capacity /= 2;
		unsigned int newCapacity = (*stackHandle)->capacity;

		(*stackHandle) = realloc((*stackHandle), (*stackHandle)->capacity * sizeof(Toy_Value) + sizeof(Toy_Stack));

		if ((*stackHandle) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reallocate a 'Toy_Stack' of %d capacity (%d space in memory)\n" TOY_CC_RESET, (int)newCapacity, (int)(newCapacity * sizeof(Toy_Value) + sizeof(Toy_Stack)));
			exit(1);
		}
	}

	return ((Toy_Value*)((*stackHandle) + 1))[--(*stackHandle)->count];
}
