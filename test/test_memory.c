#include "memory.h"

#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test single pointer
		int* integer = ALLOCATE(int, 1);

		FREE(int, integer);

		if (getAllocatedMemoryCount() != 0) {
			fprintf(stderr, ERROR "ERROR: integer failed to be allocated and freed correctly\n" RESET);
			return -1;
		}
	}

	{
		//test single pointer
		int* array = ALLOCATE(int, 10);

		FREE_ARRAY(int, array, 10);

		if (getAllocatedMemoryCount() != 0) {
			fprintf(stderr, ERROR "ERROR: integer array failed to be allocated and freed correctly\n" RESET);
			return -1;
		}
	}

	{
		//test single pointer
		int* array1 = ALLOCATE(int, 10);
		int* array2 = ALLOCATE(int, 10);

		FREE_ARRAY(int, array1, 10);
		FREE_ARRAY(int, array2, 10);

		if (getAllocatedMemoryCount() != 0) {
			fprintf(stderr, ERROR "ERROR: integer array failed to be allocated and freed correctly\n" RESET);
			return -1;
		}
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

