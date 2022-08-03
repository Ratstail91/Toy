#pragma once

#include "common.h"
#include "token_types.h"

//lexers are bound to a string of code, and return a single token every time scan is called
typedef struct {
	char* source;
	int start; //start of the token
	int current; //current position of the lexer
	int line; //track this for error handling
} Lexer;

//tokens are intermediaries between lexers and parsers
typedef struct {
	TokenType type;
	char* lexeme;
	int length;
	int line;
} Token;

void initLexer(Lexer* lexer, char* source);
Token scanLexer(Lexer* lexer);

