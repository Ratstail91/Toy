#include "literal.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test a single null literal
		Literal literal = TO_NULL_LITERAL;

		if (!IS_NULL(literal)) {
			fprintf(stderr, ERROR "ERROR: null literal failed\n" RESET);
			return -1;
		}
	}

	{
		//test boolean literals
		Literal t = TO_BOOLEAN_LITERAL(true);
		Literal f = TO_BOOLEAN_LITERAL(false);

		if (!IS_TRUTHY(t) || IS_TRUTHY(f)) {
			fprintf(stderr, ERROR "ERROR: boolean literal failed\n" RESET);
			return -1;
		}
	}

	{
		//test string literals
		char* buffer = "Hello world";

		Literal literal = TO_STRING_LITERAL(createRefString(buffer));

		freeLiteral(literal);
	}

	{
		//test identifier literals
		char buffer[] = "Hello world";

		Literal literal = TO_IDENTIFIER_LITERAL(createRefString(buffer));

		freeLiteral(literal);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}
