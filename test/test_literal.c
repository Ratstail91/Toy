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
		char* buffer = ALLOCATE(char, 128);

		snprintf(buffer, 128, "Hello world");

		Literal literal = TO_STRING_LITERAL(buffer, 128);

		freeLiteral(literal);
	}

	{
		//test identifier literals
		char* buffer = ALLOCATE(char, 128);

		snprintf(buffer, 128, "foobar");

		Literal literal = TO_IDENTIFIER_LITERAL(buffer, 128);

		freeLiteral(literal);
	}

	//check allocated memory
	if (getAllocatedMemoryCount() != 0) {
		fprintf(stderr, ERROR "ERROR: Dangling memory detected: %d byes\n" RESET, getAllocatedMemoryCount());
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}