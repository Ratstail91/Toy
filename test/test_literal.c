#include "toy_literal.h"

#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>

int main() {
	{
		//test a single null literal
		Toy_Literal literal = TOY_TO_NULL_LITERAL;

		if (!TOY_IS_NULL(literal)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: null literal failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	{
		//test boolean literals
		Toy_Literal t = TOY_TO_BOOLEAN_LITERAL(true);
		Toy_Literal f = TOY_TO_BOOLEAN_LITERAL(false);

		if (!TOY_IS_TRUTHY(t) || TOY_IS_TRUTHY(f)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: boolean literal failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	{
		//test string literals
		char* buffer = "Hello world";

		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefString(buffer));

		Toy_freeLiteral(literal);
	}

	{
		//test identifier literals
		char buffer[] = "Hello world";

		Toy_Literal literal = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(buffer));

		Toy_freeLiteral(literal);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
