#include "memory.h"

#include "console_colors.h"

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
		int* integer = ALLOCATE(int, 1);
		FREE(int, integer);
	}

	{
		//test single pointer array
		int* array = ALLOCATE(int, 10);

		array[1] = 42; //access the given memory

		FREE_ARRAY(int, array, 10);
	}

	{
		//test multiple pointer arrays
		int* array1 = ALLOCATE(int, 10);
		int* array2 = ALLOCATE(int, 10);

		array1[1] = 42; //access the given memory
		array2[1] = 42; //access the given memory

		FREE_ARRAY(int, array1, 10);
		FREE_ARRAY(int, array2, 10);
	}
}

int main() {
	//test the default allocator
	testMemoryAllocation();

	//test the custom allocator
	setMemoryAllocator(allocator);
	testMemoryAllocation();

	if (callCount != 8) {
		fprintf(stderr, ERROR "Unexpected call count for custom allocator; was called %d times" RESET, callCount);
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}
