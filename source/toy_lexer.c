#include "toy_lexer.h"
#include "toy_console_colors.h"
#include "toy_keyword_types.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

//static generic utility functions
static void cleanLexer(Toy_Lexer* lexer) {
	lexer->source = NULL;
	lexer->start = 0;
	lexer->current = 0;
	lexer->line = 1;
	lexer->commentsEnabled = true;
}

static bool isAtEnd(Toy_Lexer* lexer) {
	return lexer->source[lexer->current] == '\0';
}

static char peek(Toy_Lexer* lexer) {
	return lexer->source[lexer->current];
}

static char peekNext(Toy_Lexer* lexer) {
	if (isAtEnd(lexer)) return '\0';
	return lexer->source[lexer->current + 1];
}

static char advance(Toy_Lexer* lexer) {
	if (isAtEnd(lexer)) {
		return '\0';
	}

	//new line
	if (lexer->source[lexer->current] == '\n') {
		lexer->line++;
	}

	lexer->current++;
	return lexer->source[lexer->current - 1];
}

static void eatWhitespace(Toy_Lexer* lexer) {
	const char c = peek(lexer);

	switch(c) {
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			advance(lexer);
			break;

		//comments
		case '/':
			if (!lexer->commentsEnabled) {
				return;
			}

			//eat the line
			if (peekNext(lexer) == '/') {
				while (!isAtEnd(lexer) && advance(lexer) != '\n');
				break;
			}

			//eat the block
			if (peekNext(lexer) == '*') {
				advance(lexer);
				advance(lexer);
				while(!isAtEnd(lexer) && !(peek(lexer) == '*' && peekNext(lexer) == '/')) advance(lexer);
				advance(lexer);
				advance(lexer);
				break;
			}
		return;

		default:
			return;
	}

	//tail recursion
	eatWhitespace(lexer);
}

static bool isDigit(Toy_Lexer* lexer) {
	return peek(lexer) >= '0' && peek(lexer) <= '9';
}

static bool isAlpha(Toy_Lexer* lexer) {
	return
		(peek(lexer) >= 'A' && peek(lexer) <= 'Z') ||
		(peek(lexer) >= 'a' && peek(lexer) <= 'z') ||
		peek(lexer) == '_'
	;
}

static bool match(Toy_Lexer* lexer, char c) {
	if (peek(lexer) == c) {
		advance(lexer);
		return true;
	}

	return false;
}

//token generators
static Toy_Token makeErrorToken(Toy_Lexer* lexer, char* msg) {
	Toy_Token token;

	token.type = TOY_TOKEN_ERROR;
	token.lexeme = msg;
	token.length = strlen(msg);
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		printf("err:");
		Toy_printToken(&token);
	}
#endif

	return token;
}

static Toy_Token makeToken(Toy_Lexer* lexer, Toy_TokenType type) {
	Toy_Token token;

	token.type = type;
	token.length = lexer->current - lexer->start;
	token.lexeme = &lexer->source[lexer->current - token.length];
	token.line = lexer->line;

#ifndef TOY_EXPORT
	//BUG #10: this shows TOKEN_EOF twice due to the overarching structure of the program - can't be fixed
	if (Toy_commandLine.verbose) {
		printf("tok:");
		Toy_printToken(&token);
	}
#endif

	return token;
}

static Toy_Token makeIntegerOrFloat(Toy_Lexer* lexer) {
	Toy_TokenType type = TOY_TOKEN_LITERAL_INTEGER; //what am I making?

	while(isDigit(lexer) || peek(lexer) == '_') advance(lexer);

	if (peek(lexer) == '.' && (peekNext(lexer) >= '0' && peekNext(lexer) <= '9')) { //BUGFIX: peekNext == digit
		type = TOY_TOKEN_LITERAL_FLOAT;
		advance(lexer);
		while(isDigit(lexer) || peek(lexer) == '_') advance(lexer);
	}

	Toy_Token token;

	token.type = type;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		if (type == TOY_TOKEN_LITERAL_INTEGER) {
			printf("int:");
		} else {
			printf("flt:");
		}
		Toy_printToken(&token);
	}
#endif

	return token;
}

static bool isEscapableCharacter(char c) {
	switch (c) {
		case 'n':
		case 't':
		case '\\':
		case '"':
			return true;

		default:
			return false;
	}
}

static Toy_Token makeString(Toy_Lexer* lexer, char terminator) {
	while (!isAtEnd(lexer)) {
		//stop if you've hit the terminator
		if (peek(lexer) == terminator) {
			advance(lexer); //eat terminator
			break;
		}

		//skip escaped control characters
		if (peek(lexer) == '\\' && isEscapableCharacter(peekNext(lexer))) {
			advance(lexer);
			advance(lexer);
			continue;
		}

		//otherwise
		advance(lexer);
	}

	if (isAtEnd(lexer)) {
		return makeErrorToken(lexer, "Unterminated string");
	}

	Toy_Token token;

	token.type = TOY_TOKEN_LITERAL_STRING;
	token.lexeme = &lexer->source[lexer->start + 1];
	token.length = lexer->current - lexer->start - 2;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		printf("str:");
		Toy_printToken(&token);
	}
#endif

	return token;
}

static Toy_Token makeKeywordOrIdentifier(Toy_Lexer* lexer) {
	advance(lexer); //first letter can only be alpha

	while(isDigit(lexer) || isAlpha(lexer)) {
		advance(lexer);
	}

	//scan for a keyword
	for (int i = 0; Toy_keywordTypes[i].keyword; i++) {
		if (strlen(Toy_keywordTypes[i].keyword) == (long unsigned int)(lexer->current - lexer->start) && !strncmp(Toy_keywordTypes[i].keyword, &lexer->source[lexer->start], lexer->current - lexer->start)) {
			Toy_Token token;

			token.type = Toy_keywordTypes[i].type;
			token.lexeme = &lexer->source[lexer->start];
			token.length = lexer->current - lexer->start;
			token.line = lexer->line;

#ifndef TOY_EXPORT
			if (Toy_commandLine.verbose) {
				printf("kwd:");
				Toy_printToken(&token);
			}
#endif

			return token;
		}
	}

	//return an identifier
	Toy_Token token;

	token.type = TOY_TOKEN_IDENTIFIER;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		printf("idf:");
		Toy_printToken(&token);
	}
#endif

	return token;
}

//exposed functions
void Toy_initLexer(Toy_Lexer* lexer, const char* source) {
	cleanLexer(lexer);

	lexer->source = source;
}

Toy_Token Toy_scanLexer(Toy_Lexer* lexer) {
	eatWhitespace(lexer);

	lexer->start = lexer->current;

	if (isAtEnd(lexer)) return makeToken(lexer, TOY_TOKEN_EOF);

	if (isDigit(lexer)) return makeIntegerOrFloat(lexer);
	if (isAlpha(lexer)) return makeKeywordOrIdentifier(lexer);

	char c = advance(lexer);

	switch(c) {
		case '(': return makeToken(lexer, TOY_TOKEN_PAREN_LEFT);
		case ')': return makeToken(lexer, TOY_TOKEN_PAREN_RIGHT);
		case '{': return makeToken(lexer, TOY_TOKEN_BRACE_LEFT);
		case '}': return makeToken(lexer, TOY_TOKEN_BRACE_RIGHT);
		case '[': return makeToken(lexer, TOY_TOKEN_BRACKET_LEFT);
		case ']': return makeToken(lexer, TOY_TOKEN_BRACKET_RIGHT);

		case '+': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_PLUS_ASSIGN : match(lexer, '+') ? TOY_TOKEN_PLUS_PLUS: TOY_TOKEN_PLUS);
		case '-': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_MINUS_ASSIGN : match(lexer, '-') ? TOY_TOKEN_MINUS_MINUS: TOY_TOKEN_MINUS);
		case '*': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_MULTIPLY_ASSIGN : TOY_TOKEN_MULTIPLY);
		case '/': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_DIVIDE_ASSIGN : TOY_TOKEN_DIVIDE);
		case '%': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_MODULO_ASSIGN : TOY_TOKEN_MODULO);

		case '!': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_NOT_EQUAL : TOY_TOKEN_NOT);
		case '=': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_EQUAL : TOY_TOKEN_ASSIGN);

		case '<': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_LESS_EQUAL : TOY_TOKEN_LESS);
		case '>': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_GREATER_EQUAL : TOY_TOKEN_GREATER);

		case '&': //TOKEN_AND not used
			if (advance(lexer) != '&') {
				return makeErrorToken(lexer, "Unexpected '&'");
			} else {
				return makeToken(lexer, TOY_TOKEN_AND);
			}

		case '|':  return makeToken(lexer, match(lexer, '|') ? TOY_TOKEN_OR : TOY_TOKEN_PIPE);

		case '?': return makeToken(lexer, TOY_TOKEN_QUESTION);
		case ':': return makeToken(lexer, TOY_TOKEN_COLON);
		case ';': return makeToken(lexer, TOY_TOKEN_SEMICOLON);
		case ',': return makeToken(lexer, TOY_TOKEN_COMMA);
		case '.':
			if (peek(lexer) == '.' && peekNext(lexer) == '.') {
				advance(lexer);
				advance(lexer);
				return makeToken(lexer, TOY_TOKEN_REST);
			}
			return makeToken(lexer, TOY_TOKEN_DOT);

		case '"':
			return makeString(lexer, c);
			//TODO: possibly support interpolated strings

		default: {
			char buffer[128];
			snprintf(buffer, 128, "Unexpected token: %c", c);
			return makeErrorToken(lexer, buffer);
		}
	}
}

static void trim(char** s, int* l) { //all this to remove a newline?
	while( isspace(( (*((unsigned char**)(s)))[(*l) - 1] )) ) (*l)--;
	while(**s && isspace( **(unsigned char**)(s)) ) { (*s)++; (*l)--; }
}

//for debugging
void Toy_printToken(Toy_Token* token) {
	if (token->type == TOY_TOKEN_ERROR) {
		printf(TOY_CC_ERROR "Error\t%d\t%.*s\n" TOY_CC_RESET, token->line, token->length, token->lexeme);
		return;
	}

	printf("\t%d\t%d\t", token->type, token->line);

	if (token->type == TOY_TOKEN_IDENTIFIER || token->type == TOY_TOKEN_LITERAL_INTEGER || token->type == TOY_TOKEN_LITERAL_FLOAT || token->type == TOY_TOKEN_LITERAL_STRING) {
		printf("%.*s\t", token->length, token->lexeme);
	} else {
		char* keyword = Toy_findKeywordByType(token->type);

		if (keyword != NULL) {
			printf("%s", keyword);
		} else {
			char* str = (char*)token->lexeme; //strip const-ness for trimming
			int length = token->length;
			trim(&str, &length);
			printf("%.*s", length, str);
		}
	}

	printf("\n");
}

void Toy_private_setComments(Toy_Lexer* lexer, bool enabled) {
	lexer->commentsEnabled = enabled;
}