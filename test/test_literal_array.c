#include "toy_literal_array.h"

#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & cleanup
		Toy_LiteralArray array;
		Toy_initLiteralArray(&array);
		Toy_freeLiteralArray(&array);
	}

	{
		//test pushing and pulling
		Toy_LiteralArray array;
		Toy_initLiteralArray(&array);

		for (int i = 0; i < 100; i++) {
			Toy_pushLiteralArray(&array, TOY_TO_INTEGER_LITERAL(i));
		}

		for (int i = 0; i < 90; i++) {
			Toy_Literal lit = Toy_popLiteralArray(&array);

			Toy_freeLiteral(lit);
		}

		if (array.count != 10) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Array didn't clear the correct number of literal integers\n" TOY_CC_RESET);
			Toy_freeLiteralArray(&array);
			return -1;
		}

		Toy_freeLiteralArray(&array);
	}

	{
		//check string, identifier and compound type behaviours
		Toy_LiteralArray array;
		Toy_initLiteralArray(&array);

		//raw
		char* str_raw = "hello world";
		char* idn_raw = "foobar";

		Toy_Literal string = TOY_TO_STRING_LITERAL(Toy_createRefString(str_raw));
		Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(idn_raw));

		//[string : string]
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, false);
		TOY_TYPE_PUSH_SUBTYPE(&type, TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, false));
		TOY_TYPE_PUSH_SUBTYPE(&type, TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, false));

		//push
		Toy_pushLiteralArray(&array, string);
		Toy_pushLiteralArray(&array, identifier);
		Toy_pushLiteralArray(&array, type);

		//free the local literals
		Toy_freeLiteral(string);
		Toy_freeLiteral(identifier);
		Toy_freeLiteral(type);

		Toy_freeLiteralArray(&array);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
