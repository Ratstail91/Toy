#include "toy_value.h"
#include "toy_console_colors.h"

#include <stdio.h>

int main() {
	//test for the correct size
	{
		if (sizeof(Toy_Value) != 8) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_Value' is an unexpected size in memory\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating a null
	{
		Toy_Value v = TOY_VALUE_TO_NULL();

		if (!TOY_VALUE_IS_NULL(v)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: creating a 'null' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating values
	{
		Toy_Value t = TOY_VALUE_TO_BOOLEAN(true);
		Toy_Value f = TOY_VALUE_TO_BOOLEAN(false);

		if (!TOY_VALUE_IS_TRUTHY(t) || TOY_VALUE_IS_TRUTHY(f)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'boolean' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test value equality
	{
		Toy_Value answer = TOY_VALUE_TO_INTEGER(42);
		Toy_Value question = TOY_VALUE_TO_INTEGER(42);
		Toy_Value nice = TOY_VALUE_TO_INTEGER(69);

		if (!TOY_VALUE_IS_EQUAL(answer, question)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: equality check failed, expected true\n" TOY_CC_RESET);
			return -1;
		}

		if (TOY_VALUE_IS_EQUAL(answer, nice)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: equality check failed, expected false\n" TOY_CC_RESET);
			return -1;
		}
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
