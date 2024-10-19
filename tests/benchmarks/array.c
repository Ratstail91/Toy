#include "toy_array.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
//	for (int i = 0; i < argc; i++) {
//		printf("argv[%d]: %s\n", i, argv[i]);
//	}

//	if (iargc != 2) return -1;

	unsigned int iterations = atoi(argv[1]);

//	printf("Found %d iterations\n", iterations);

	Toy_Array* array = TOY_ARRAY_ALLOCATE();

	for (int i = 0; i < iterations; i++) {
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(i));
	}

	TOY_ARRAY_FREE(array);

	return 0;
}
