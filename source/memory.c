#include "memory.h"

#include "console_colors.h"

#include <stdio.h>
#include <stdlib.h>

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0 && oldSize == 0) {
		//causes issues, so just skip out with a NO-OP
		return NULL;
	}

	if (newSize == 0) {
		free(pointer);

		return NULL;
	}

	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		fprintf(stderr, ERROR "[internal]Memory allocation error (requested %d for %ld, replacing %d)\n" ERROR, (int)newSize, (long int)pointer, (int)oldSize);
		exit(-1);
	}

	return mem;
}

