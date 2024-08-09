#include "toy_memory.h"
#include "toy_refstring.h"
#include "toy_reffunction.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//default allocator
void* Toy_private_defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize) {
	//causes issues, so just skip out with a NO-OP (DISABLED for performance reasons)
	// if (newSize == 0 && oldSize == 0) {
	// 	return NULL;
	// }

	if (newSize == 0) {
		free(pointer);
		return NULL;
	}

	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] Memory allocation error (requested %d, replacing %d)\n" TOY_CC_RESET, (int)newSize, (int)oldSize);
		return NULL;
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
	Toy_setRefFunctionAllocatorFn(fn);
}
