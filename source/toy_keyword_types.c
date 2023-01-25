#include "toy_keyword_types.h"

#include "toy_common.h"

#include <string.h>

Toy_KeywordType Toy_keywordTypes[] = {
	//type keywords
	{TOY_TOKEN_NULL,       "null"},
	{TOY_TOKEN_BOOLEAN,    "bool"},
	{TOY_TOKEN_INTEGER,    "int"},
	{TOY_TOKEN_FLOAT,      "float"},
	{TOY_TOKEN_STRING,     "string"},
	{TOY_TOKEN_FUNCTION,   "fn"},
	{TOY_TOKEN_OPAQUE,     "opaque"},
	{TOY_TOKEN_ANY,        "any"},

	//other keywords
	{TOY_TOKEN_AS,         "as"},
	{TOY_TOKEN_ASSERT,     "assert"},
	{TOY_TOKEN_BREAK,      "break"},
	{TOY_TOKEN_CLASS,      "class"},
	{TOY_TOKEN_CONST,      "const"},
	{TOY_TOKEN_CONTINUE,   "continue"},
	{TOY_TOKEN_DO,         "do"},
	{TOY_TOKEN_ELSE,       "else"},
	{TOY_TOKEN_EXPORT,     "export"},
	{TOY_TOKEN_FOR,        "for"},
	{TOY_TOKEN_FOREACH,    "foreach"},
	{TOY_TOKEN_IF,         "if"},
	{TOY_TOKEN_IMPORT,     "import"},
	{TOY_TOKEN_IN,         "in"},
	{TOY_TOKEN_OF,         "of"},
	{TOY_TOKEN_PRINT,      "print"},
	{TOY_TOKEN_RETURN,     "return"},
	{TOY_TOKEN_TYPE,       "type"},
	{TOY_TOKEN_ASTYPE,     "astype"},
	{TOY_TOKEN_TYPEOF,     "typeof"},
	{TOY_TOKEN_VAR,        "var"},
	{TOY_TOKEN_WHILE,      "while"},

	//literal values
	{TOY_TOKEN_LITERAL_TRUE,   "true"},
	{TOY_TOKEN_LITERAL_FALSE,  "false"},

	//meta tokens
	{TOY_TOKEN_PASS,       NULL},
	{TOY_TOKEN_ERROR,      NULL},

	{TOY_TOKEN_EOF, NULL},
};

char* Toy_findKeywordByType(Toy_TokenType type) {
	if (type == TOY_TOKEN_EOF) {
		return "EOF";
	}

	for(int i = 0; Toy_keywordTypes[i].keyword; i++) {
		if (Toy_keywordTypes[i].type == type) {
			return Toy_keywordTypes[i].keyword;
		}
	}

	return NULL;
}

Toy_TokenType Toy_findTypeByKeyword(const char* keyword) {
	const int length = strlen(keyword);

	for (int i = 0; Toy_keywordTypes[i].keyword; i++) {
		if (!strncmp(keyword, Toy_keywordTypes[i].keyword, length)) {
			return Toy_keywordTypes[i].type;
		}
	}

	return TOY_TOKEN_EOF;
}
