#include "toy_value.h"

#include "toy_console_colors.h"

#include <stdio.h>

bool Toy_private_isTruthy(Toy_Value value) {
	//null is an error
	if (TOY_VALUE_IS_NULL(value)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: 'null' is neither true nor false\n" TOY_CC_RESET);
		return false;
	}

	//only 'false' is falsy
	if (TOY_VALUE_IS_BOOLEAN(value)) {
		return TOY_VALUE_AS_BOOLEAN(value);
	}

	//anything else is truthy
	return true;
}