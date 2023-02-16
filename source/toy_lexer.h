#pragma once

#include "toy_common.h"
#include "toy_token_types.h"

//lexers are bound to a string of code, and return a single token every time scan is called
typedef struct {
	const char* source;
	int start; //start of the token
	int current; //current position of the lexer
	int line; //track this for error handling
	bool commentsEnabled; //BUGFIX: enable comments (disabled in repl)
} Toy_Lexer;

//tokens are intermediaries between lexers and parsers
typedef struct {
	Toy_TokenType type;
	const char* lexeme;
	int length;
	int line;
} Toy_Token;

TOY_API void Toy_initLexer(Toy_Lexer* lexer, const char* source);
TOY_API Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer);

//for debugging
TOY_API void Toy_private_printToken(Toy_Token* token);

TOY_API void Toy_private_setComments(Toy_Lexer* lexer, bool enabled);
