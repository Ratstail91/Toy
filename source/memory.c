#include "memory.h"

#include "console_colors.h"

#include <stdio.h>
#include <stdlib.h>

static int allocatedMemoryCount = 0;

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	allocatedMemoryCount -= oldSize;
	allocatedMemoryCount += newSize;

	if (newSize == 0) {
		free(pointer);

		return NULL;
	}

	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		fprintf(stderr, ERROR "[internal]Memory allocation error (requested %d for %d, replacing %d)\n" ERROR, (int)newSize, (int)pointer, (int)oldSize);
		exit(-1);
	}

	return mem;
}

int getAllocatedMemoryCount() {
	return allocatedMemoryCount;
}