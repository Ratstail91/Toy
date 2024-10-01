#include "toy_array.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_resizeArray() {
	//test single pointer
	{
		Toy_Array* array = TOY_ALLOCATE_ARRAY(int, 1);
		TOY_FREE_ARRAY(int, array);
	}

	//test single pointer array
	{
		Toy_Array* array = TOY_ALLOCATE_ARRAY(int, 10);

		//check you can access the memory
		array->data[1] = 42;

		TOY_FREE_ARRAY(int, array);
	}

	//test multiple pointer arrays
	{
		Toy_Array* array1 = TOY_ALLOCATE_ARRAY(int, 10);
		Toy_Array* array2 = TOY_ALLOCATE_ARRAY(int, 10);

		array1->data[1] = 42; //access the given memory
		array2->data[1] = 42; //access the given memory

		TOY_FREE_ARRAY(int, array1);
		TOY_FREE_ARRAY(int, array2);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_resizeArray();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	return total;
}
