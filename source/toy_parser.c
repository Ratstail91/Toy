#include "toy_parser.h"
#include "toy_console_colors.h"

#include <stdio.h>

//utilities
static void printError(Toy_Parser* parser, Toy_Token token, const char* errorMsg) {
	//keep going while panicking
	if (parser->panic) {
		return;
	}

	fprintf(stderr, TOY_CC_ERROR "[Line %d] Error ", (int)token.line);

	//check type
	if (token.type == TOY_TOKEN_EOF) {
		fprintf(stderr, "at end");
	}
	else {
		fprintf(stderr, "at '%.*s'", (int)token.length, token.lexeme);
	}

	//finally
	fprintf(stderr, ": %s\n" TOY_CC_RESET, errorMsg);
	parser->error = true;
	parser->panic = true;
}

static void advance(Toy_Parser* parser) {
	parser->previous = parser->current;
	parser->current = Toy_private_scanLexer(parser->lexer);

	if (parser->current.type == TOY_TOKEN_ERROR) {
		printError(parser, parser->current, "Can't read the source code");
	}
}

static bool match(Toy_Parser* parser, Toy_TokenType tokenType) {
	if (parser->current.type == tokenType) {
		advance(parser);
		return true;
	}
	return false;
}

static void consume(Toy_Parser* parser, Toy_TokenType tokenType, const char* msg) {
	if (parser->current.type != tokenType) {
		printError(parser, parser->current, msg);
		return;
	}

	advance(parser);
}

static void synchronize(Toy_Parser* parser) {
	while (parser->current.type != TOY_TOKEN_EOF) {
		switch(parser->current.type) {
			//these tokens can start a statement
			case TOY_TOKEN_KEYWORD_ASSERT:
			case TOY_TOKEN_KEYWORD_BREAK:
			case TOY_TOKEN_KEYWORD_CLASS:
			case TOY_TOKEN_KEYWORD_CONTINUE:
			case TOY_TOKEN_KEYWORD_DO:
			case TOY_TOKEN_KEYWORD_EXPORT:
			case TOY_TOKEN_KEYWORD_FOR:
			case TOY_TOKEN_KEYWORD_FOREACH:
			case TOY_TOKEN_KEYWORD_FUNCTION:
			case TOY_TOKEN_KEYWORD_IF:
			case TOY_TOKEN_KEYWORD_IMPORT:
			case TOY_TOKEN_KEYWORD_PRINT:
			case TOY_TOKEN_KEYWORD_RETURN:
			case TOY_TOKEN_KEYWORD_VAR:
			case TOY_TOKEN_KEYWORD_WHILE:
				parser->error = true;
				parser->panic = false;
				return;

			default:
				advance(parser);
		}
	}
}

//precedence declarations
typedef enum ParsingPrecedence {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_GROUP,
	PREC_TERNARY,
	PREC_OR,
	PREC_AND,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} ParsingPrecedence;

typedef Toy_AstFlag (*ParsingRule)(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);

typedef struct ParsingTuple {
	ParsingPrecedence precedence;
	ParsingRule prefix;
	ParsingRule infix;
} ParsingTuple;

static void parsePrecedence(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, ParsingPrecedence precRule);

static Toy_AstFlag atomic(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag unary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag binary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag group(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);

//precedence definitions
static ParsingTuple parsingRulesetTable[] = {
	{PREC_PRIMARY,atomic,NULL},// TOY_TOKEN_NULL,

	//variable names
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_IDENTIFIER,

	//types
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_TYPE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_BOOLEAN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_INTEGER,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_FLOAT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_STRING,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_ARRAY,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_DICTIONARY,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_FUNCTION,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_OPAQUE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_ANY,

	//keywords and reserved words
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_AS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_ASSERT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_BREAK,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CLASS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CONST,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CONTINUE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_DO,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_ELSE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_EXPORT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FOR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FOREACH,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FUNCTION,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IF,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IMPORT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_OF,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_PRINT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_RETURN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_TYPEAS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_TYPEOF,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_VAR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_WHILE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_YIELD,

	//literal values
	{PREC_PRIMARY,atomic,NULL},// TOY_TOKEN_LITERAL_TRUE,
	{PREC_PRIMARY,atomic,NULL},// TOY_TOKEN_LITERAL_FALSE,
	{PREC_PRIMARY,atomic,NULL},// TOY_TOKEN_LITERAL_INTEGER,
	{PREC_PRIMARY,atomic,NULL},// TOY_TOKEN_LITERAL_FLOAT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_LITERAL_STRING,

	//math operators
	{PREC_TERM,NULL,binary},// TOY_TOKEN_OPERATOR_ADD,
	{PREC_TERM,unary,binary},// TOY_TOKEN_OPERATOR_SUBTRACT,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_MULTIPLY,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_DIVIDE,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_MODULO,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_ADD_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_MODULO_ASSIGN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_INCREMENT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_DECREMENT,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_ASSIGN,

	//comparator operators
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_EQUAL,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_NOT,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_LESS,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_LESS_EQUAL,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_GREATER,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_GREATER_EQUAL,

	//structural operators
	{PREC_NONE,group,NULL},// TOY_TOKEN_OPERATOR_PAREN_LEFT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_PAREN_RIGHT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACKET_LEFT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACKET_RIGHT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACE_LEFT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACE_RIGHT,

	//other operators
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_AND,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_OR,
	{PREC_NONE,unary,NULL},// TOY_TOKEN_OPERATOR_NEGATE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_QUESTION,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_COLON,

	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_CONCAT, // ..
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_REST, // ...

	//unused operators
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_AMPERSAND, // &
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_PIPE, // |

	//meta tokens
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_PASS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_ERROR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_EOF,
};

static Toy_AstFlag atomic(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	switch(parser->previous.type) {
		case TOY_TOKEN_NULL:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_NULL());
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_TRUE:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_BOOLEAN(true));
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_FALSE:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_BOOLEAN(false));
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_INTEGER: {
			//filter the '_' character
			char buffer[parser->previous.length];

			unsigned int i = 0, o = 0;
			do {
				buffer[i] = parser->previous.lexeme[o];
				if (buffer[i] != '_') i++;
			} while (parser->previous.lexeme[o++] && i < parser->previous.length);
			buffer[i] = '\0'; //BUGFIX

			int value = 0;
			sscanf(buffer, "%d", &value);
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_INTEGER(value));
			return TOY_AST_FLAG_NONE;
		}

		case TOY_TOKEN_LITERAL_FLOAT: {
			//filter the '_' character
			char buffer[parser->previous.length];

			unsigned int i = 0, o = 0;
			do {
				buffer[i] = parser->previous.lexeme[o];
				if (buffer[i] != '_') i++;
			} while (parser->previous.lexeme[o++] && i < parser->previous.length);
			buffer[i] = '\0'; //BUGFIX

			float value = 0;
			sscanf(buffer, "%f", &value);
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_FLOAT(value));
			return TOY_AST_FLAG_NONE;
		}

		default:
			printError(parser, parser->previous, "Unexpected token passed to atomic precedence rule");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag unary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//'subtract' can only be applied to numbers and groups, while 'negate' can only be applied to booleans and groups
	//this function takes the libery of peeking into the uppermost node, to see if it can apply this to it

	if (parser->previous.type == TOY_TOKEN_OPERATOR_SUBTRACT) {

		bool connectedDigit = parser->previous.lexeme[1] >= '0' && parser->previous.lexeme[1] <= '9'; //BUGFIX: '- 1' should not be optimised into a negative
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_UNARY);

		//negative numbers
		if ((*rootHandle)->type == TOY_AST_VALUE && TOY_VALUE_IS_INTEGER((*rootHandle)->value.value) && connectedDigit) {
			(*rootHandle)->value.value = TOY_VALUE_FROM_INTEGER( -TOY_VALUE_AS_INTEGER((*rootHandle)->value.value) );
		}
		else if ((*rootHandle)->type == TOY_AST_VALUE && TOY_VALUE_IS_FLOAT((*rootHandle)->value.value) && connectedDigit) {
			(*rootHandle)->value.value = TOY_VALUE_FROM_FLOAT( -TOY_VALUE_AS_FLOAT((*rootHandle)->value.value) );
		}
		else {
			//actually emit the negation node
			Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_NEGATE);
		}
	}

	else if (parser->previous.type == TOY_TOKEN_OPERATOR_NEGATE) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_UNARY);

		//inverted booleans
		if ((*rootHandle)->type == TOY_AST_VALUE && TOY_VALUE_IS_BOOLEAN((*rootHandle)->value.value)) {
			(*rootHandle)->value.value = TOY_VALUE_FROM_BOOLEAN( !TOY_VALUE_AS_BOOLEAN((*rootHandle)->value.value) );
		}
		else {
			//actually emit the negation node
			Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_NEGATE);
		}
	}

	else {
		printError(parser, parser->previous, "Unexpected token passed to unary precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
	}

	return TOY_AST_FLAG_NONE;
}

static Toy_AstFlag binary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//infix must advance
	advance(parser);

	switch(parser->previous.type) {
		//arithmetic
		case TOY_TOKEN_OPERATOR_ADD: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_TERM + 1);
			return TOY_AST_FLAG_ADD;
		}

		case TOY_TOKEN_OPERATOR_SUBTRACT: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_TERM + 1);
			return TOY_AST_FLAG_SUBTRACT;
		}

		case TOY_TOKEN_OPERATOR_MULTIPLY: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_MULTIPLY;
		}

		case TOY_TOKEN_OPERATOR_DIVIDE: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_DIVIDE;
		}

		case TOY_TOKEN_OPERATOR_MODULO: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_MODULO;
		}

		//assignment
		case TOY_TOKEN_OPERATOR_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_ADD_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_ADD_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_SUBTRACT_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_MULTIPLY_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_DIVIDE_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_MODULO_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_MODULO_ASSIGN;
		}

		//comparison
		case TOY_TOKEN_OPERATOR_COMPARE_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_EQUAL;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_NOT: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_NOT;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_LESS: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_LESS;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_LESS_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_LESS_EQUAL;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_GREATER: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_GREATER;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_GREATER_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_GREATER_EQUAL;
		}

		default:
			printError(parser, parser->previous, "Unexpected token passed to binary precedence rule");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag group(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//groups are ()
	if (parser->previous.type == TOY_TOKEN_OPERATOR_PAREN_LEFT) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP);
		consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' at end of group");

		//Toy_AstGroup is omitted from generation, as an optimisation
		// Toy_private_emitAstGroup(bucketHandle, rootHandle);
	}

	else {
		printError(parser, parser->previous, "Unexpected token passed to grouping precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
	}

	return TOY_AST_FLAG_NONE;
}

static ParsingTuple* getParsingRule(Toy_TokenType type) {
	return &parsingRulesetTable[type];
}

//grammar rules
static void parsePrecedence(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, ParsingPrecedence precRule) {
	//'step over' the token to parse
	advance(parser);

	//every valid expression has a prefix rule
	ParsingRule prefix = getParsingRule(parser->previous.type)->prefix;

	if (prefix == NULL) {
		printError(parser, parser->previous, "Expected expression");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	prefix(bucketHandle, parser, rootHandle);

	//infix rules are left-recursive
	while (precRule <= getParsingRule(parser->current.type)->precedence) {
		ParsingRule infix = getParsingRule(parser->current.type)->infix;

		if (infix == NULL) {
			printError(parser, parser->previous, "Expected operator");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return;
		}

		Toy_Ast* ptr = NULL;
		Toy_AstFlag flag = infix(bucketHandle, parser, &ptr);

		//finished
		if (flag == TOY_AST_FLAG_NONE) {
			(*rootHandle) = ptr;
			return;
		}

		Toy_private_emitAstBinary(bucketHandle, rootHandle, flag, ptr);
	}

	//can't assign below a certain precedence
	if (precRule <= PREC_ASSIGNMENT && match(parser, TOY_TOKEN_OPERATOR_ASSIGN)) {
		printError(parser, parser->current, "Invalid assignment target");
	}
}

static void makeExpr(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT);
}

static void makePrintStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	makeExpr(bucketHandle, parser, rootHandle);
	Toy_private_emitAstPrint(bucketHandle, rootHandle);

	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of print statement");
}

static void makeExprStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	makeExpr(bucketHandle, parser, rootHandle);
	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of expression statement");
}

static void makeStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//block
	//assert
	//if-then-else
	//while-then
	//for-pre-clause-post-then
	//break
	//continue
	//return
	//import

	//check for empty lines
	if (match(parser, TOY_TOKEN_OPERATOR_SEMICOLON)) {
		Toy_private_emitAstPass(bucketHandle, rootHandle);
		return;
	}

	else if (match(parser, TOY_TOKEN_KEYWORD_PRINT)) {
		makePrintStmt(bucketHandle, parser, rootHandle);
		return;
	}

	else {
		//default
		makeExprStmt(bucketHandle, parser, rootHandle);
		return;
	}
}

static void makeDeclarationStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	// //variable declarations
	// if (match(parser, TOY_TOKEN_KEYWORD_VAR)) {
	// 	makeVariableDeclarationStmt(bucketHandle, parser, rootHandle);
	// }

	// //function declarations
	// else if (match(parser, TOY_TOKEN_KEYWORD_FUNCTION)) {
	// 	makeFunctionDeclarationStmt(bucketHandle, parser, rootHandle);
	// }

	//otherwise
	// else {
		makeStmt(bucketHandle, parser, rootHandle);
	// }
}

static void makeBlockStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//begin the block
	Toy_private_initAstBlock(bucketHandle, rootHandle);

	//read a series of statements into the block
	while (!match(parser, TOY_TOKEN_EOF)) {
		//process the grammar rules
		Toy_Ast* stmt = NULL;
		makeDeclarationStmt(bucketHandle, parser, &stmt);

		//if something went wrong
		if (parser->panic) {
			synchronize(parser);

			Toy_Ast* err = NULL;
			Toy_private_emitAstError(bucketHandle, &err);
			Toy_private_appendAstBlock(bucketHandle, *rootHandle, err);

			continue;
		}
		Toy_private_appendAstBlock(bucketHandle, *rootHandle, stmt);
	}
}

//exposed functions
void Toy_bindParser(Toy_Parser* parser, Toy_Lexer* lexer) {
	Toy_resetParser(parser);
	parser->lexer = lexer;
	advance(parser);
}

Toy_Ast* Toy_scanParser(Toy_Bucket** bucketHandle, Toy_Parser* parser) {
	Toy_Ast* rootHandle = NULL;

	//check for EOF
	if (match(parser, TOY_TOKEN_EOF)) {
		Toy_private_emitAstEnd(bucketHandle, &rootHandle);
		return rootHandle;
	}

	makeBlockStmt(bucketHandle, parser, &rootHandle);

	return rootHandle;
}

void Toy_resetParser(Toy_Parser* parser) {
	parser->lexer = NULL;

	parser->current = TOY_BLANK_TOKEN();
	parser->previous = TOY_BLANK_TOKEN();

	parser->error = false;
	parser->panic = false;
}
