#include "toy_memory.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, newSize);

	if (result == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Memory allocation error (requested %d, replacing %d)\n" TOY_CC_RESET, (int)newSize, (int)oldSize);
		exit(1);
	}

	return result;
}