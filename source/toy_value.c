#include "toy_value.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

bool Toy_private_isTruthy(Toy_Value value) {
	//null is an error
	if (TOY_VALUE_IS_NULL(value)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: 'null' is neither true nor false\n" TOY_CC_RESET);
		exit(-1); //TODO: return false or exit()?
	}

	//only 'false' is falsy
	if (TOY_VALUE_IS_BOOLEAN(value)) {
		return TOY_VALUE_AS_BOOLEAN(value);
	}

	//anything else is truthy
	return true;
}

bool Toy_private_isEqual(Toy_Value left, Toy_Value right) {
	//temp check
	if (right.type > TOY_VALUE_FLOAT) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown types %d and %d in equality\n" TOY_CC_RESET, left.type, right.type);
		exit(-1);
	}

	switch(left.type) {
		case TOY_VALUE_NULL:
			return TOY_VALUE_IS_NULL(right);

		case TOY_VALUE_BOOLEAN:
			return TOY_VALUE_IS_BOOLEAN(right) && TOY_VALUE_AS_BOOLEAN(left) == TOY_VALUE_AS_BOOLEAN(right);

		case TOY_VALUE_INTEGER:
			if (TOY_VALUE_AS_INTEGER(right)) {
				return TOY_VALUE_AS_INTEGER(left) == TOY_VALUE_AS_INTEGER(right);
			}
			if (TOY_VALUE_AS_FLOAT(right)) {
				return TOY_VALUE_AS_INTEGER(left) == TOY_VALUE_AS_FLOAT(right);
			}
			return false;

		case TOY_VALUE_FLOAT:
			if (TOY_VALUE_AS_FLOAT(right)) {
				return TOY_VALUE_AS_FLOAT(left) == TOY_VALUE_AS_FLOAT(right);
			}
			if (TOY_VALUE_AS_INTEGER(right)) {
				return TOY_VALUE_AS_FLOAT(left) == TOY_VALUE_AS_INTEGER(right);
			}
			return false;

		case TOY_VALUE_STRING:
		case TOY_VALUE_ARRAY:
		case TOY_VALUE_DICTIONARY:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		default:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown types %d and %d in equality\n" TOY_CC_RESET, left.type, right.type);
			exit(-1);
	}
}
