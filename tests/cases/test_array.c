#include "toy_array.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_array() {
	//test allocation and free
	{
		Toy_Array* array = Toy_resizeArray(NULL, 1);
		array = Toy_resizeArray(array, 0);
	}

	//test initial data
	{
		Toy_Array* array = Toy_resizeArray(NULL, 10);

		//check you can access the memory
		array->data[1] = TOY_VALUE_FROM_INTEGER(42);

		Toy_resizeArray(array, 0);
	}

	//test multiple arrays (no overlaps or conflicts)
	{
		Toy_Array* array1 = Toy_resizeArray(NULL, 10);
		Toy_Array* array2 = Toy_resizeArray(NULL, 10);

		array1->data[1] = TOY_VALUE_FROM_INTEGER(42);
		array2->data[1] = TOY_VALUE_FROM_INTEGER(42);

		Toy_resizeArray(array1, 0);
		Toy_resizeArray(array2, 0);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_array();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	return total;
}
