#include "lexer.h"
#include "console_colors.h"
#include "keyword_types.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

//static generic utility functions
static void cleanLexer(Lexer* lexer) {
	lexer->source = NULL;
	lexer->start = 0;
	lexer->current = 0;
	lexer->line = 1;
}

static bool isAtEnd(Lexer* lexer) {
	return lexer->source[lexer->current] == '\0';
}

static char peek(Lexer* lexer) {
	return lexer->source[lexer->current];
}

static char peekNext(Lexer* lexer) {
	if (isAtEnd(lexer)) return '\0';
	return lexer->source[lexer->current + 1];
}

static char advance(Lexer* lexer) {
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

static void eatWhitespace(Lexer* lexer) {
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
			//eat the line
			if (peekNext(lexer) == '/') {
				while (advance(lexer) != '\n' && !isAtEnd(lexer));
				break;
			}

			//eat the block
			if (peekNext(lexer) == '*') {
				advance(lexer);
				advance(lexer);
				while(!(peek(lexer) == '*' && peekNext(lexer) == '/')) advance(lexer);
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

static bool isDigit(Lexer* lexer) {
	return peek(lexer) >= '0' && peek(lexer) <= '9';
}

static bool isAlpha(Lexer* lexer) {
	return
		(peek(lexer) >= 'A' && peek(lexer) <= 'Z') ||
		(peek(lexer) >= 'a' && peek(lexer) <= 'z') ||
		peek(lexer) == '_'
	;
}

static bool match(Lexer* lexer, char c) {
	if (peek(lexer) == c) {
		advance(lexer);
		return true;
	}

	return false;
}

//token generators
static Token makeErrorToken(Lexer* lexer, char* msg) {
	Token token;

	token.type = TOKEN_ERROR;
	token.lexeme = msg;
	token.length = strlen(msg);
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (command.verbose) {
		printf("err:");
		printToken(&token);
	}
#endif

	return token;
}

static Token makeToken(Lexer* lexer, TokenType type) {
	Token token;

	token.type = type;
	token.length = lexer->current - lexer->start;
	token.lexeme = &lexer->source[lexer->current - token.length];
	token.line = lexer->line;

#ifndef TOY_EXPORT
	//BUG #10: this shows TOKEN_EOF twice due to the overarching structure of the program - can't be fixed
	if (command.verbose) {
		printf("tok:");
		printToken(&token);
	}
#endif

	return token;
}

static Token makeIntegerOrFloat(Lexer* lexer) {
	TokenType type = TOKEN_LITERAL_INTEGER; //what am I making?

	while(isDigit(lexer)) advance(lexer);

	if (peek(lexer) == '.' && (peekNext(lexer) >= '0' && peekNext(lexer) <= '9')) { //BUGFIX: peekNext == digit
		type = TOKEN_LITERAL_FLOAT;
		advance(lexer);
		while(isDigit(lexer)) advance(lexer);
	}

	Token token;

	token.type = type;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (command.verbose) {
		if (type == TOKEN_LITERAL_INTEGER) {
			printf("int:");
		} else {
			printf("flt:");
		}
		printToken(&token);
	}
#endif

	return token;
}

static Token makeString(Lexer* lexer, char terminator) {
	while (!isAtEnd(lexer) && peek(lexer) != terminator) {
		advance(lexer);
	}

	advance(lexer); //eat terminator

	if (isAtEnd(lexer)) {
		return makeErrorToken(lexer, "Unterminated string");
	}

	Token token;

	token.type = TOKEN_LITERAL_STRING;
	token.lexeme = &lexer->source[lexer->start + 1];
	token.length = lexer->current - lexer->start - 2;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (command.verbose) {
		printf("str:");
		printToken(&token);
	}
#endif

	return token;
}

static Token makeKeywordOrIdentifier(Lexer* lexer) {
	advance(lexer); //first letter can only be alpha

	while(isDigit(lexer) || isAlpha(lexer)) {
		advance(lexer);
	}

	//scan for a keyword
	for (int i = 0; keywordTypes[i].keyword; i++) {
		if (strlen(keywordTypes[i].keyword) == (long unsigned int)(lexer->current - lexer->start) && !strncmp(keywordTypes[i].keyword, &lexer->source[lexer->start], lexer->current - lexer->start)) {
			Token token;

			token.type = keywordTypes[i].type;
			token.lexeme = &lexer->source[lexer->start];
			token.length = lexer->current - lexer->start;
			token.line = lexer->line;

#ifndef TOY_EXPORT
			if (command.verbose) {
				printf("kwd:");
				printToken(&token);
			}
#endif

			return token;
		}
	}

	//return an identifier
	Token token;

	token.type = TOKEN_IDENTIFIER;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

#ifndef TOY_EXPORT
	if (command.verbose) {
		printf("idf:");
		printToken(&token);
	}
#endif

	return token;
}

//exposed functions
void initLexer(Lexer* lexer, char* source) {
	cleanLexer(lexer);

	lexer->source = source;
}

Token scanLexer(Lexer* lexer) {
	eatWhitespace(lexer);

	lexer->start = lexer->current;

	if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);

	if (isDigit(lexer)) return makeIntegerOrFloat(lexer);
	if (isAlpha(lexer)) return makeKeywordOrIdentifier(lexer);

	char c = advance(lexer);

	switch(c) {
		case '(': return makeToken(lexer, TOKEN_PAREN_LEFT);
		case ')': return makeToken(lexer, TOKEN_PAREN_RIGHT);
		case '{': return makeToken(lexer, TOKEN_BRACE_LEFT);
		case '}': return makeToken(lexer, TOKEN_BRACE_RIGHT);
		case '[': return makeToken(lexer, TOKEN_BRACKET_LEFT);
		case ']': return makeToken(lexer, TOKEN_BRACKET_RIGHT);

		case '+': return makeToken(lexer, match(lexer, '=') ? TOKEN_PLUS_ASSIGN : match(lexer, '+') ? TOKEN_PLUS_PLUS: TOKEN_PLUS);
		case '-': return makeToken(lexer, match(lexer, '=') ? TOKEN_MINUS_ASSIGN : match(lexer, '-') ? TOKEN_MINUS_MINUS: TOKEN_MINUS);
		case '*': return makeToken(lexer, match(lexer, '=') ? TOKEN_MULTIPLY_ASSIGN : TOKEN_MULTIPLY);
		case '/': return makeToken(lexer, match(lexer, '=') ? TOKEN_DIVIDE_ASSIGN : TOKEN_DIVIDE);
		case '%': return makeToken(lexer, match(lexer, '=') ? TOKEN_MODULO_ASSIGN : TOKEN_MODULO);

		case '!': return makeToken(lexer, match(lexer, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
		case '=': return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUAL : TOKEN_ASSIGN);

		case '<': return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>': return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

		case '&': //TOKEN_AND not used
			if (advance(lexer) != '&') {
				return makeErrorToken(lexer, "Unexpected '&'");
			} else {
				return makeToken(lexer, TOKEN_AND);
			}

		case '|':  return makeToken(lexer, match(lexer, '|') ? TOKEN_OR : TOKEN_PIPE);

		case ':': return makeToken(lexer, TOKEN_COLON);
		case ';': return makeToken(lexer, TOKEN_SEMICOLON);
		case ',': return makeToken(lexer, TOKEN_COMMA);
		case '.':
			if (peek(lexer) == '.' && peekNext(lexer) == '.') {
				advance(lexer);
				advance(lexer);
				return makeToken(lexer, TOKEN_REST);
			}
			return makeToken(lexer, TOKEN_DOT);

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
void printToken(Token* token) {
	if (token->type == TOKEN_ERROR) {
		printf(ERROR "Error\t%d\t%.*s\n" RESET, token->line, token->length, token->lexeme);
		return;
	}

	printf("\t%d\t%d\t", token->type, token->line);

	if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_LITERAL_INTEGER || token->type == TOKEN_LITERAL_FLOAT || token->type == TOKEN_LITERAL_STRING) {
		printf("%.*s\t", token->length, token->lexeme);
	} else {
		char* keyword = findKeywordByType(token->type);

		if (keyword != NULL) {
			printf("%s", keyword);
		} else {
			char* str = token->lexeme;
			int length = token->length;
			trim(&str, &length);
			printf("%.*s", length, str);
		}
	}

	printf("\n");
}
