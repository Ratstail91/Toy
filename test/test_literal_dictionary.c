#include "toy_literal_dictionary.h"

#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & cleanup
		Toy_LiteralDictionary dictionary;
		Toy_initLiteralDictionary(&dictionary);
		Toy_freeLiteralDictionary(&dictionary);
	}

	{
		//test insertion and deletion
		char* idn_raw = "foobar";
		char* str_raw = "hello world";

		Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(idn_raw));
		Toy_Literal string = TOY_TO_STRING_LITERAL(Toy_createRefString(str_raw));

		Toy_LiteralDictionary dictionary;
		Toy_initLiteralDictionary(&dictionary);

		Toy_setLiteralDictionary(&dictionary, identifier, string);

		Toy_freeLiteral(identifier);
		Toy_freeLiteral(string);

		Toy_freeLiteralDictionary(&dictionary);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

