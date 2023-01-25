#pragma once

#include "toy_common.h"
#include "toy_token_types.h"

//lexers are bound to a string of code, and return a single token every time scan is called
typedef struct {
	char* source;
	int start; //start of the token
	int current; //current position of the lexer
	int line; //track this for error handling
} Toy_Lexer;

//tokens are intermediaries between lexers and parsers
typedef struct {
	Toy_TokenType type;
	char* lexeme;
	int length;
	int line;
} Toy_Token;

TOY_API void Toy_initLexer(Toy_Lexer* lexer, char* source);
Toy_Token Toy_scanLexer(Toy_Lexer* lexer);

//for debugging
void Toy_printToken(Toy_Token* token);
