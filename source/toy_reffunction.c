#include "toy_reffunction.h"

#include <string.h>

//memory allocation
extern void* Toy_private_defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize);
static Toy_RefFunctionAllocatorFn allocate = Toy_private_defaultMemoryAllocator;

void Toy_setRefFunctionAllocatorFn(Toy_RefFunctionAllocatorFn allocator) {
	allocate = allocator;
}

//API
Toy_RefFunction* Toy_createRefFunction(const void* data, size_t length) {
	//allocate the memory area (including metadata space)
	Toy_RefFunction* refFunction = allocate(NULL, 0, sizeof(size_t) + sizeof(int) + sizeof(char) * length);

	if (refFunction == NULL) {
		return NULL;
	}

	//set the data
	refFunction->refCount = 1;
	refFunction->length = length;
	memcpy(refFunction->data, data, refFunction->length);

	return refFunction;
}

void Toy_deleteRefFunction(Toy_RefFunction* refFunction) {
	//decrement, then check
	refFunction->refCount--;
	if (refFunction->refCount <= 0) {
		allocate(refFunction, sizeof(size_t) + sizeof(int) + sizeof(char) * (refFunction->length + 1), 0);
	}
}

int Toy_countRefFunction(Toy_RefFunction* refFunction) {
	return refFunction->refCount;
}

size_t Toy_lengthRefFunction(Toy_RefFunction* refFunction) {
	return refFunction->length;
}

Toy_RefFunction* Toy_copyRefFunction(Toy_RefFunction* refFunction) {
	//Cheaty McCheater Face
	refFunction->refCount++;
	return refFunction;
}

Toy_RefFunction* Toy_deepCopyRefFunction(Toy_RefFunction* refFunction) {
	//create a new function, with a new refCount
	return Toy_createRefFunction(refFunction->data, refFunction->length);
}
