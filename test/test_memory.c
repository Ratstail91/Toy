#include "toy_memory.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//the test allocator
static int callCount = 0;

void* allocator(void* pointer, size_t oldSize, size_t newSize) {
	callCount++;

	if (newSize == 0 && oldSize == 0) {
		return NULL;
	}

	if (newSize == 0) {
		free(pointer);
		return NULL;
	}

	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		exit(-1);
	}

	return mem;
}

void testMemoryAllocation() {
	{
		//test single pointer
		int* integer = TOY_ALLOCATE(int, 1);
		TOY_FREE(int, integer);
	}

	{
		//test single pointer array
		int* array = TOY_ALLOCATE(int, 10);

		array[1] = 42; //access the given memory

		TOY_FREE_ARRAY(int, array, 10);
	}

	{
		//test multiple pointer arrays
		int* array1 = TOY_ALLOCATE(int, 10);
		int* array2 = TOY_ALLOCATE(int, 10);

		array1[1] = 42; //access the given memory
		array2[1] = 42; //access the given memory

		TOY_FREE_ARRAY(int, array1, 10);
		TOY_FREE_ARRAY(int, array2, 10);
	}
}

int main() {
	//test the default allocator
	testMemoryAllocation();

	//test the custom allocator
	Toy_setMemoryAllocator(allocator);
	testMemoryAllocation();

	if (callCount != 8) {
		fprintf(stderr, TOY_CC_ERROR "Unexpected call count for custom allocator; was called %d times" TOY_CC_RESET, callCount);
		return -1;
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
