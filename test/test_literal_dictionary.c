#include "literal_dictionary.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & cleanup
		LiteralDictionary dictionary;
		initLiteralDictionary(&dictionary);
		freeLiteralDictionary(&dictionary);
	}

	{
		//test insertion and deletion
		char* idn_raw = "foobar";
		char* str_raw = "hello world";

		Literal identifier = TO_IDENTIFIER_LITERAL(copyString(idn_raw, strlen(idn_raw)), strlen(idn_raw));
		Literal string = TO_STRING_LITERAL(copyString(str_raw, strlen(str_raw)), strlen(str_raw));

		LiteralDictionary dictionary;
		initLiteralDictionary(&dictionary);

		setLiteralDictionary(&dictionary, identifier, string);

		freeLiteral(identifier);
		freeLiteral(string);

		freeLiteralDictionary(&dictionary);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

