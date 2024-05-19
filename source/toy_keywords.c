#include "toy_keywords.h"

#include <string.h>

const Toy_KeywordTypeTuple Toy_private_keywords[] = {
	//null
	{TOY_TOKEN_NULL, "null"},

	//types
	{TOY_TOKEN_TYPE_TYPE, "type"},
	{TOY_TOKEN_TYPE_BOOLEAN, "bool"},
	{TOY_TOKEN_TYPE_INTEGER, "int"},
	{TOY_TOKEN_TYPE_FLOAT, "float"},
	{TOY_TOKEN_TYPE_STRING, "string"},
	// TOY_TOKEN_TYPE_ARRAY,
	// TOY_TOKEN_TYPE_DICTIONARY,
	{TOY_TOKEN_TYPE_FUNCTION, "fn"},
	{TOY_TOKEN_TYPE_OPAQUE, "opaque"},
	{TOY_TOKEN_TYPE_ANY, "any"},

	//keywords and reserved words
	{TOY_TOKEN_KEYWORD_AS, "as"},
	{TOY_TOKEN_KEYWORD_ASSERT, "assert"},
	{TOY_TOKEN_KEYWORD_BREAK, "break"},
	{TOY_TOKEN_KEYWORD_CLASS, "class"},
	{TOY_TOKEN_KEYWORD_CONST, "const"}, //TODO: investigate the constness of types
	{TOY_TOKEN_KEYWORD_CONTINUE, "continue"},
	{TOY_TOKEN_KEYWORD_DO, "do"},
	{TOY_TOKEN_KEYWORD_ELSE, "else"},
	{TOY_TOKEN_KEYWORD_EXPORT, "export"},
	{TOY_TOKEN_KEYWORD_FOR, "for"},
	{TOY_TOKEN_KEYWORD_FOREACH, "foreach"},
	{TOY_TOKEN_KEYWORD_IF, "if"},
	{TOY_TOKEN_KEYWORD_IMPORT, "import"},
	{TOY_TOKEN_KEYWORD_IN, "in"},
	{TOY_TOKEN_KEYWORD_OF, "of"},
	{TOY_TOKEN_KEYWORD_PRINT, "print"},
	{TOY_TOKEN_KEYWORD_RETURN, "return"},
	{TOY_TOKEN_KEYWORD_TYPEAS, "typeas"},
	{TOY_TOKEN_KEYWORD_TYPEOF, "typeof"},
	{TOY_TOKEN_KEYWORD_VAR, "var"},
	{TOY_TOKEN_KEYWORD_WHILE, "while"},

	//literal values
	{TOY_TOKEN_LITERAL_TRUE, "true"},
	{TOY_TOKEN_LITERAL_FALSE, "false"},

	{TOY_TOKEN_EOF, NULL},
};

const char* Toy_private_findKeywordByType(const Toy_TokenType type) {
	if (type == TOY_TOKEN_EOF) {
		return "EOF";
	}

	for(int i = 0; Toy_private_keywords[i].keyword; i++) {
		if (Toy_private_keywords[i].type == type) {
			return Toy_private_keywords[i].keyword;
		}
	}

	return NULL;
}

Toy_TokenType Toy_private_findTypeByKeyword(const char* keyword) {
	const int length = strlen(keyword);

	for (int i = 0; Toy_private_keywords[i].keyword; i++) {
		if (!strncmp(keyword, Toy_private_keywords[i].keyword, length)) {
			return Toy_private_keywords[i].type;
		}
	}

	return TOY_TOKEN_EOF;
}
