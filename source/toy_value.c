#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>

bool Toy_private_isTruthy(Toy_Value value) {
	//null is an error
	if (TOY_VALUE_IS_NULL(value)) {
		Toy_error(TOY_CC_ERROR "ERROR: 'null' is neither true nor false\n" TOY_CC_RESET);
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
	if (right.type > TOY_VALUE_STRING) {
		Toy_error(TOY_CC_ERROR "ERROR: Unknown types in value equality comparison\n" TOY_CC_RESET);
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
			if (TOY_VALUE_IS_STRING(right)) {
				return Toy_compareStrings(TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right)) == 0;
			}
			return false;

		case TOY_VALUE_ARRAY:
		case TOY_VALUE_DICTIONARY:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		default:
		Toy_error(TOY_CC_ERROR "ERROR: Unknown types in value equality comparison\n" TOY_CC_RESET);
	}

	return 0;
}

//hash util
static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

unsigned int Toy_hashValue(Toy_Value value) {
	switch(value.type) {
		case TOY_VALUE_NULL:
			return 0;

		case TOY_VALUE_BOOLEAN:
			return TOY_VALUE_AS_BOOLEAN(value) ? 1 : 0;

		case TOY_VALUE_INTEGER:
			return hashUInt(TOY_VALUE_AS_INTEGER(value));

		case TOY_VALUE_FLOAT:
			return hashUInt( *((int*)(&TOY_VALUE_AS_FLOAT(value))) );

		case TOY_VALUE_STRING:
			return Toy_hashString(TOY_VALUE_AS_STRING(value));

		case TOY_VALUE_ARRAY:
		case TOY_VALUE_DICTIONARY:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		default:
			Toy_error(TOY_CC_ERROR "ERROR: Can't hash an unknown type\n" TOY_CC_RESET);
	}

	return 0;
}
