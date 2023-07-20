#pragma once

/*!
# toy_lexer.h

This header defines the lexer and token structures, which can be bound to a piece of source code, and used to tokenize it within a parser.
!*/

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

/*!
## Defined Functions
!*/

/*!
### void Toy_initLexer(Toy_Lexer* lexer, const char* source)

This function initializes a lexer, binding it to the `source` parameter; the lexer is now ready to be passed to the parser.
!*/
TOY_API void Toy_initLexer(Toy_Lexer* lexer, const char* source);

/*!
### Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer)

This function "scans" the lexer, returning a token to the parser.

Private functions are not intended for general use.
!*/
TOY_API Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer);

/*!
### void Toy_private_printToken(Toy_Token* token)

This function prints a given token to stdout.

Private functions are not intended for general use.
!*/
TOY_API void Toy_private_printToken(Toy_Token* token);

/*!
### void Toy_private_setComments(Toy_Lexer* lexer, bool enabled)

This function sets whether comments are allowed within source code. By default, comments are allowed, and are only disabled in the repl.

Private functions are not intended for general use.
!*/
TOY_API void Toy_private_setComments(Toy_Lexer* lexer, bool enabled);
