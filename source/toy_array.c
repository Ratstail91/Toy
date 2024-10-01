#include "toy_array.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

Toy_Array* Toy_resizeArray(Toy_Array* paramArray, size_t capacity) {
	if (capacity == 0) {
		free(paramArray);
		return NULL;
	}

	Toy_Array* array = realloc(paramArray, capacity + sizeof(Toy_Array));

	if (array == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Array' of %d capacity\n" TOY_CC_RESET, (int)capacity);
		exit(1);
	}

	array->capacity = capacity;
	array->count = paramArray == NULL ? 0 :
		(array->count > capacity ? capacity : array->count); //truncate lost data

	return array;
}
