#include "parser.h"
#include "common.h"

#include "memory.h"
#include "literal.h"

#include <stdio.h>

//utility functions
static void error(Parser* parser, Token token, const char* message) {
	//keep going while panicing
	if (parser->panic) return;

	fprintf(stderr, "[Line %d] Error", token.line);

	//check type
	if (token.type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	}

	else {
		fprintf(stderr, " at '%.*s'", token.length, token.lexeme);
	}

	//finally
	fprintf(stderr, ": %s\n", message);
	parser->error = true;
	parser->panic = true;
}

static void advance(Parser* parser) {
	parser->previous = parser->current;
	parser->current = scanLexer(parser->lexer);

	if (parser->current.type == TOKEN_ERROR) {
		error(parser, parser->current, "Lexer error");
	}
}

static bool match(Parser* parser, TokenType tokenType) {
	if (parser->current.type == tokenType) {
		advance(parser);
		return true;
	}
	return false;
}

static void consume(Parser* parser, TokenType tokenType, const char* msg) {
	if (parser->current.type != tokenType) {
		error(parser, parser->current, msg);
		return;
	}

	advance(parser);
}

static void synchronize(Parser* parser) {
	while (parser->current.type != TOKEN_EOF) {
		switch(parser->current.type) {
			//these tokens can start a line
			case TOKEN_ASSERT:
			case TOKEN_BREAK:
			case TOKEN_CONST:
			case TOKEN_CONTINUE:
			case TOKEN_DO:
			case TOKEN_EXPORT:
			case TOKEN_FOR:
			case TOKEN_FOREACH:
			case TOKEN_IF:
			case TOKEN_IMPORT:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
			case TOKEN_VAR:
			case TOKEN_WHILE:
				parser->panic = false;
				return;

			default:
				advance(parser);
		}
	}
}

//the pratt table collates the precedence rules
typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_TERNARY,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} PrecedenceRule;

typedef void (*ParseFn)(Parser* parser, Node** nodeHandle, bool canBeAssigned);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	PrecedenceRule precedence;
} ParseRule;

ParseRule parseRules[];

//forward declarations
static void parsePrecedence(Parser* parser, Node** nodeHandle, PrecedenceRule rule);

//the atomic expression rules
static void string(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//handle strings
	switch(parser->previous.type) {
		case TOKEN_LITERAL_STRING:
			emitAtomicLiteral(nodeHandle, TO_STRING_LITERAL(copyString(parser->previous.lexeme, parser->previous.length)));
		break;

		//TODO: interpolated strings

		default:
			error(parser, parser->previous, "Unexpected token passed to string precedence rule");
	}
}

static void binary(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//TODO
}

static void unary(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//TODO
}

static void atomic(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	switch(parser->previous.type) {
		case TOKEN_NULL:
			emitAtomicLiteral(nodeHandle, TO_NULL_LITERAL);
		break;

		case TOKEN_LITERAL_TRUE:
			emitAtomicLiteral(nodeHandle, TO_BOOLEAN_LITERAL(true));
		break;

		case TOKEN_LITERAL_FALSE:
			emitAtomicLiteral(nodeHandle, TO_BOOLEAN_LITERAL(false));
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to atomic precedence rule");
	}
}

ParseRule parseRules[] = { //must match the token types
	//types
	{atomic, NULL, PREC_NONE},// TOKEN_NULL,
	{NULL, NULL, PREC_NONE},// TOKEN_BOOLEAN,
	{NULL, NULL, PREC_NONE},// TOKEN_INTEGER,
	{NULL, NULL, PREC_NONE},// TOKEN_FLOAT,
	{NULL, NULL, PREC_NONE},// TOKEN_STRING,
	{NULL, NULL, PREC_NONE},// TOKEN_ARRAY,
	{NULL, NULL, PREC_NONE},// TOKEN_DICTIONARY,
	{NULL, NULL, PREC_NONE},// TOKEN_FUNCTION,
	{NULL, NULL, PREC_NONE},// TOKEN_ANY,

	//keywords and reserved words
	{NULL, NULL, PREC_NONE},// TOKEN_AS,
	{NULL, NULL, PREC_NONE},// TOKEN_ASSERT,
	{NULL, NULL, PREC_NONE},// TOKEN_BREAK,
	{NULL, NULL, PREC_NONE},// TOKEN_CLASS,
	{NULL, NULL, PREC_NONE},// TOKEN_CONST,
	{NULL, NULL, PREC_NONE},// TOKEN_CONTINUE,
	{NULL, NULL, PREC_NONE},// TOKEN_DO,
	{NULL, NULL, PREC_NONE},// TOKEN_ELSE,
	{NULL, NULL, PREC_NONE},// TOKEN_EXPORT,
	{NULL, NULL, PREC_NONE},// TOKEN_FOR,
	{NULL, NULL, PREC_NONE},// TOKEN_FOREACH,
	{NULL, NULL, PREC_NONE},// TOKEN_IF,
	{NULL, NULL, PREC_NONE},// TOKEN_IMPORT,
	{NULL, NULL, PREC_NONE},// TOKEN_IN,
	{NULL, NULL, PREC_NONE},// TOKEN_OF,
	{NULL, NULL, PREC_NONE},// TOKEN_PRINT,
	{NULL, NULL, PREC_NONE},// TOKEN_RETURN,
	{NULL, NULL, PREC_NONE},// TOKEN_USING,
	{NULL, NULL, PREC_NONE},// TOKEN_VAR,
	{NULL, NULL, PREC_NONE},// TOKEN_WHILE,

	//literal values
	{NULL, NULL, PREC_NONE},// TOKEN_IDENTIFIER,
	{atomic, NULL, PREC_NONE},// TOKEN_LITERAL_TRUE,
	{atomic, NULL, PREC_NONE},// TOKEN_LITERAL_FALSE,
	{NULL, NULL, PREC_NONE},// TOKEN_LITERAL_INTEGER,
	{NULL, NULL, PREC_NONE},// TOKEN_LITERAL_FLOAT,
	{string, NULL, PREC_PRIMARY},// TOKEN_LITERAL_STRING,

	//math operators
	{NULL, NULL, PREC_NONE},// TOKEN_PLUS,
	{NULL, NULL, PREC_NONE},// TOKEN_MINUS,
	{NULL, NULL, PREC_NONE},// TOKEN_MULTIPLY,
	{NULL, NULL, PREC_NONE},// TOKEN_DIVIDE,
	{NULL, NULL, PREC_NONE},// TOKEN_MODULO,
	{NULL, NULL, PREC_NONE},// TOKEN_PLUS_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MINUS_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MULTIPLY_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_DIVIDE_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MODULO_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_PLUS_PLUS,
	{NULL, NULL, PREC_NONE},// TOKEN_MINUS_MINUS,

	//logical operators
	{NULL, NULL, PREC_NONE},// TOKEN_PAREN_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_PAREN_RIGHT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACKET_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACKET_RIGHT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACE_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACE_RIGHT,
	{NULL, NULL, PREC_NONE},// TOKEN_NOT,
	{NULL, NULL, PREC_NONE},// TOKEN_NOT_EQUAL,
	{NULL, NULL, PREC_NONE},// TOKEN_EQUAL,
	{NULL, NULL, PREC_NONE},// TOKEN_LESS,
	{NULL, NULL, PREC_NONE},// TOKEN_GREATER,
	{NULL, NULL, PREC_NONE},// TOKEN_LESS_EQUAL,
	{NULL, NULL, PREC_NONE},// TOKEN_GREATER_EQUAL,
	{NULL, NULL, PREC_NONE},// TOKEN_AND,
	{NULL, NULL, PREC_NONE},// TOKEN_OR,

	//other operators
	{NULL, NULL, PREC_NONE},// TOKEN_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_COLON,
	{NULL, NULL, PREC_NONE},// TOKEN_SEMICOLON,
	{NULL, NULL, PREC_NONE},// TOKEN_COMMA,
	{NULL, NULL, PREC_NONE},// TOKEN_DOT,
	{NULL, NULL, PREC_NONE},// TOKEN_PIPE,
	{NULL, NULL, PREC_NONE},// TOKEN_REST,

	//meta tokens
	{NULL, NULL, PREC_NONE},// TOKEN_PASS,
	{NULL, NULL, PREC_NONE},// TOKEN_ERROR,
	{NULL, NULL, PREC_NONE},// TOKEN_EOF,
};

ParseRule* getRule(TokenType type) {
	return &parseRules[type];
}

static void parsePrecedence(Parser* parser, Node** nodeHandle, PrecedenceRule rule) {
	//every expression has a prefix rule
	advance(parser);
	ParseFn prefixRule = getRule(parser->previous.type)->prefix;

	if (prefixRule == NULL) {
		error(parser, parser->previous, "Expected expression");
		return;
	}

	bool canBeAssigned = rule <= PREC_ASSIGNMENT;
	prefixRule(parser, nodeHandle, canBeAssigned);

	//infix rules are left-recursive
	while (rule <= getRule(parser->current.type)->precedence) {
		ParseFn infixRule = getRule(parser->current.type)->infix;

		if (infixRule == NULL) {
			error(parser, parser->current, "Expected operator");
			return;
		}

		infixRule(parser, nodeHandle, canBeAssigned); //NOTE: infix rule must advance the parser
	}

	//if your precedence is below "assignment"
	if (canBeAssigned && match(parser, TOKEN_ASSIGN)) {
		error(parser, parser->current, "Invalid assignment target");
	}
}

//expressions
static void expression(Parser* parser, Node** nodeHandle) {
	//delegate to the pratt table for expression precedence
	parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
}

//statements
static void printStmt(Parser* parser, Node* node) {
	int line = parser->previous.line;
	
	//set the node info
	node->type = NODE_UNARY;
	node->unary.child = ALLOCATE(Node, 1);
	expression(parser, &(node->unary.child));

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of print statement");
}

//precedence functions
static void expressionStmt(Parser* parser, Node* node) {
	error(parser, parser->previous, "Expression statements not yet implemented");
}

static void statement(Parser* parser, Node* node) {
	//print
	if (match(parser, TOKEN_PRINT)) {
		printStmt(parser, node);
		return;
	}

	//default
	expressionStmt(parser, node);
}

static void declaration(Parser* parser, Node* node) {
	statement(parser, node);

	if (parser->panic) {
		synchronize(parser);
	}
}

//exposed functions
void initParser(Parser* parser, Lexer* lexer) {
	parser->lexer = lexer;
	parser->error = false;
	parser->panic = false;

	parser->previous.type = TOKEN_NULL;
	parser->current.type = TOKEN_NULL;
	advance(parser);
}

void freeParser(Parser* parser) {
	initParser(parser, NULL);
}

Node* scanParser(Parser* parser) {
	//check for EOF
	if (match(parser, TOKEN_EOF)) {
		return NULL;
	}

	//returns nodes on the heap
	Node* node = ALLOCATE(Node, 1);

	//process the grammar rule for this line
	declaration(parser, node);

	return node;
}

