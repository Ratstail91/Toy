#include "literal_array.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & cleanup
		LiteralArray array;
		initLiteralArray(&array);
		freeLiteralArray(&array);
	}

	{
		//test pushing and pulling
		LiteralArray array;
		initLiteralArray(&array);

		for (int i = 0; i < 100; i++) {
			pushLiteralArray(&array, TO_INTEGER_LITERAL(i));
		}

		for (int i = 0; i < 90; i++) {
			Literal lit = popLiteralArray(&array);

			freeLiteral(lit);
		}

		if (array.count != 10) {
			fprintf(stderr, ERROR "ERROR: Array didn't clear the correct number of literal integers\n" RESET);
			freeLiteralArray(&array);
			return -1;
		}

		freeLiteralArray(&array);
	}

	{
		//check string, identifier and compound type behaviours
		LiteralArray array;
		initLiteralArray(&array);

		//raw
		char* str_raw = "hello world";
		char* idn_raw = "foobar";

		Literal string = TO_STRING_LITERAL(copyString(str_raw, strlen(str_raw)), strlen(str_raw));
		Literal identifier = TO_IDENTIFIER_LITERAL(copyString(idn_raw, strlen(idn_raw)), strlen(idn_raw));

		//[string, string]
		Literal type = TO_TYPE_LITERAL(LITERAL_DICTIONARY, false);
		TYPE_PUSH_SUBTYPE(&type, TO_TYPE_LITERAL(LITERAL_STRING, false));
		TYPE_PUSH_SUBTYPE(&type, TO_TYPE_LITERAL(LITERAL_STRING, false));

		//push
		pushLiteralArray(&array, string);
		pushLiteralArray(&array, identifier);
		pushLiteralArray(&array, type);

		//free the local literals
		freeLiteral(string);
		freeLiteral(identifier);
		freeLiteral(type);

		freeLiteralArray(&array);
	}

	//check allocated memory
	if (getAllocatedMemoryCount() != 0) {
		fprintf(stderr, ERROR "ERROR: Dangling memory detected: %d byes\n" RESET, getAllocatedMemoryCount());
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}