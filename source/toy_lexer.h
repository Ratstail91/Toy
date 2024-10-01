#pragma once

#include "toy_token_types.h"
#include "toy_common.h"

//lexers are bound to a string of code
typedef struct {
	size_t start; //start of the current token
	size_t current; //current position of the lexer
	size_t line; //track this for error handling
	const char* source;
} Toy_Lexer;

//tokens are intermediaries between lexers and parsers
typedef struct {
	Toy_TokenType type;
	size_t length;
	size_t line;
	const char* lexeme;
} Toy_Token;

TOY_API void Toy_bindLexer(Toy_Lexer* lexer, const char* source);
TOY_API Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer);
TOY_API void Toy_private_printToken(Toy_Token* token); //debugging

//util
#define TOY_BLANK_TOKEN() ((Toy_Token){TOY_TOKEN_NULL, 0, 0, NULL})
