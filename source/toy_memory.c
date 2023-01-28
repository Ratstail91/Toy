#include "toy_memory.h"
#include "toy_refstring.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//default allocator
void* Toy_private_defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize) {
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
		fprintf(stderr, TOY_CC_ERROR "[internal] Memory allocation error (requested %d for %ld, replacing %d)\n" TOY_CC_RESET, (int)newSize, (long int)pointer, (int)oldSize);
		exit(-1);
	}

	return mem;
}

//static variables
static Toy_MemoryAllocatorFn allocator = Toy_private_defaultMemoryAllocator;

//exposed API
void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize) {
	return allocator(pointer, oldSize, newSize);
}

void Toy_setMemoryAllocator(Toy_MemoryAllocatorFn fn) {
	if (fn == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] Memory allocator error (can't be null)\n" TOY_CC_RESET);
		exit(-1);
	}

	if (fn == Toy_reallocate) {
		fprintf(stderr, TOY_CC_ERROR "[internal] Memory allocator error (can't loop the Toy_reallocate function)\n" TOY_CC_RESET);
		exit(-1);
	}

	allocator = fn;
	Toy_setRefStringAllocatorFn(fn);
}
