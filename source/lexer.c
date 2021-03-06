#include "lexer.h"
#include "keyword_types.h"

#include "command.h"

#include "debug.h"

#include <stdio.h>
#include <string.h>

static void cleanLexer(Lexer* lexer) {
	lexer->source = NULL;
	lexer->start = 0;
	lexer->current = 0;
	lexer->line = 1;
}

void initLexer(Lexer* lexer, char* source) {
	cleanLexer(lexer);

	lexer->source = source;
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
			if (peekNext(lexer) == '/') {
				while (advance(lexer) != '\n' && !isAtEnd(lexer));
				break;
			}

			if (peekNext(lexer) == '*') {
				advance(lexer);
				advance(lexer);
				while(!(peek(lexer) == '*' && peekNext(lexer) == '/')) advance(lexer);
				advance(lexer);
				advance(lexer);
				break;
			}

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
		peek(lexer) >= 'A' && peek(lexer) <= 'Z' ||
		peek(lexer) >= 'a' && peek(lexer) <= 'z' ||
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

static Token makeErrorToken(Lexer* lexer, char* msg) {
	Token token;

	token.type = TOKEN_ERROR;
	token.lexeme = msg;
	token.length = strlen(msg);
	token.line = lexer->line;

	if (command.verbose) {
		printf("err:");
		printToken(&token);
	}

	return token;
}

static Token makeToken(Lexer* lexer, TokenType type) {
	Token token;

	token.type = type;
	token.lexeme = &lexer->source[lexer->current - 1];
	token.length = 1;
	token.line = lexer->line;

	if (command.verbose) {
		printf("tok:");
		printToken(&token);
	}

	return token;
}

static Token makeNumber(Lexer* lexer) {
	while(isDigit(lexer)) advance(lexer);

	if (peek(lexer) == '.') {
		advance(lexer);
		while(isDigit(lexer));
	}

	Token token;

	token.type = TOKEN_NUMBER;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

	if (command.verbose) {
		printf("num:");
		printToken(&token);
	}

	return token;
}

static Token makeString(Lexer* lexer, char terminator) {
	while (!isAtEnd(lexer) && peek(lexer) != terminator) {
		//escaping strings
		if (peek(lexer) == '\\') {
			advance(lexer);
		}

		advance(lexer);
	}

	advance(lexer); //eat terminator

	if (isAtEnd(lexer)) {
		return makeErrorToken(lexer, "Unterminated string");
	}

	Token token;

	token.type = terminator == '`' ? TOKEN_INTERPOLATED_STRING : TOKEN_STRING;
	token.lexeme = &lexer->source[lexer->start + 1];
	token.length = lexer->current - lexer->start - 2;
	token.line = lexer->line;

	if (command.verbose) {
		printf("str:");
		printToken(&token);
	}

	return token;
}

static Token makeKeywordOrIdentifier(Lexer* lexer) {
	advance(lexer); //first letter can only be alpha

	while(isDigit(lexer) || isAlpha(lexer)) {
		advance(lexer);
	}

	//scan for a keyword
	for (int i = 0; keywordTypes[i].keyword; i++) {
		if (strlen(keywordTypes[i].keyword) == lexer->current - lexer->start && !strncmp(keywordTypes[i].keyword, &lexer->source[lexer->start], lexer->current - lexer->start)) {
			Token token;

			token.type = keywordTypes[i].type;
			token.lexeme = &lexer->source[lexer->start];
			token.length = lexer->current - lexer->start;
			token.line = lexer->line;

			return token;
		}
	}

	//return an identifier
	Token token;

	token.type = TOKEN_IDENTIFIER;
	token.lexeme = &lexer->source[lexer->start];
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;

	if (command.verbose) {
		printf("key:");
		printToken(&token);
	}

	return token;
}

Token scanLexer(Lexer* lexer) {
	eatWhitespace(lexer);

	lexer->start = lexer->current;

	if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);

	if (isDigit(lexer)) return makeNumber(lexer);
	if (isAlpha(lexer)) return makeKeywordOrIdentifier(lexer);

	char c = advance(lexer);

	switch(c) {
		case '(': return makeToken(lexer, TOKEN_LEFT_PAREN);
		case ')': return makeToken(lexer, TOKEN_RIGHT_PAREN);
		case '{': return makeToken(lexer, TOKEN_LEFT_BRACE);
		case '}': return makeToken(lexer, TOKEN_RIGHT_BRACE);
		case '[': return makeToken(lexer, TOKEN_LEFT_BRACKET);
		case ']': return makeToken(lexer, TOKEN_RIGHT_BRACKET);
		case ';': return makeToken(lexer, TOKEN_SEMICOLON);
		case ',': return makeToken(lexer, TOKEN_COMMA);

		case '+': return makeToken(lexer, match(lexer, '=') ? TOKEN_PLUS_EQUAL : match(lexer, '+') ? TOKEN_PLUS_PLUS: TOKEN_PLUS);
		case '-': return makeToken(lexer, match(lexer, '=') ? TOKEN_MINUS_EQUAL : match(lexer, '-') ? TOKEN_MINUS_MINUS: TOKEN_MINUS);
		case '*': return makeToken(lexer, match(lexer, '=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
		case '/': return makeToken(lexer, match(lexer, '=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
		case '%': return makeToken(lexer, match(lexer, '=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO);

		case '!': return makeToken(lexer, match(lexer, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=': return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUAL_EQUAL : match(lexer, '>') ? TOKEN_EQUAL_GREATER: TOKEN_EQUAL);

		case '>': return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		case '<': return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQUAL : match(lexer, '|') ? TOKEN_LESS_OR: TOKEN_LESS);

		case '&':
			if (advance(lexer) != '&') {
				return makeErrorToken(lexer, "Unexpected '&'");
			} else {
				return makeToken(lexer, TOKEN_AND_AND);
			}

		case '|':
			if (peek(lexer) != '|' && peek(lexer) != '>') {
				return makeErrorToken(lexer, "Unexpected '|'");
			} else {
				switch(advance(lexer)) {
					case '|': return makeToken(lexer, TOKEN_OR_OR);
					case '>': return makeToken(lexer, TOKEN_OR_GREATER);
				}
			}

		case '.':
			if (peek(lexer) == '.' && peekNext(lexer) == '.') {
				return makeToken(lexer, TOKEN_DOT_DOT_DOT);
			} else if (peek(lexer) == '.') {
				return makeToken(lexer, TOKEN_DOT_DOT);
			} else {
				return makeToken(lexer, TOKEN_DOT);
			}

		case '?': return makeToken(lexer, TOKEN_QUESTION);
		case ':': return makeToken(lexer, match(lexer, ':') ? TOKEN_COLON_COLON : TOKEN_COLON);

		case '"':
		case '`':
			return makeString(lexer, c);

		default:
			return makeErrorToken(lexer, "Unexpected token");
	}
}
