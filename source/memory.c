#include "memory.h"
#include "refstring.h"

#include "console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//default allocator
void* defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize) {
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
		fprintf(stderr, ERROR "[internal] Memory allocation error (requested %d for %ld, replacing %d)\n" RESET, (int)newSize, (long int)pointer, (int)oldSize);
		exit(-1);
	}

	return mem;
}

//static variables
static MemoryAllocatorFn allocator;

//preload
static void __attribute__((constructor)) preloadMemoryAllocator() {
	setMemoryAllocator(defaultMemoryAllocator);
}

//exposed API
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	return allocator(pointer, oldSize, newSize);
}

void setMemoryAllocator(MemoryAllocatorFn fn) {
	if (fn == NULL) {
		fprintf(stderr, ERROR "[internal] Memory allocator error (can't be null)\n" RESET);
		exit(-1);
	}

	if (fn == reallocate) {
		fprintf(stderr, ERROR "[internal] Memory allocator error (can't loop the reallocate function)\n" RESET);
		exit(-1);
	}

	allocator = fn;
	setRefStringAllocatorFn(fn);
}
