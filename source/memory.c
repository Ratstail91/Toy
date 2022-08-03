#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0) {
		free(pointer);

		return NULL;
	}

	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		fprintf(stderr, "[Internal]Memory allocation error (requested %d for %d, replacing %d)\n", (int)newSize, (int)pointer, (int)oldSize);
		exit(-1);
	}

	return mem;
}

