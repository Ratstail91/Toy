#include "parser.h"

#include "common.h"

#include "memory.h"
#include "literal.h"
#include "opcodes.h"

#include "console_colors.h"

#include <stdio.h>

//utility functions
static void error(Parser* parser, Token token, const char* message) {
	//keep going while panicing
	if (parser->panic) return;

	fprintf(stderr, ERROR "[Line %d] Error", token.line);

	//check type
	if (token.type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	}

	else {
		fprintf(stderr, " at '%.*s'", token.length, token.lexeme);
	}

	//finally
	fprintf(stderr, ": %s\n" RESET, message);
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
	if (command.verbose) {
		fprintf(stderr, ERROR "synchronizing\n" RESET);
	}

	while (parser->current.type != TOKEN_EOF) {
		switch(parser->current.type) {
			//these tokens can start a line
			case TOKEN_ASSERT:
			case TOKEN_BREAK:
			case TOKEN_CLASS:
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
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} PrecedenceRule;

typedef Opcode (*ParseFn)(Parser* parser, Node** nodeHandle);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	PrecedenceRule precedence;
} ParseRule;

ParseRule parseRules[];

//forward declarations
static void declaration(Parser* parser, Node** nodeHandle);
static void parsePrecedence(Parser* parser, Node** nodeHandle, PrecedenceRule rule);
static Literal readTypeToLiteral(Parser* parser);

//the expression rules
static Opcode asType(Parser* parser, Node** nodeHandle) {
	Literal literal = readTypeToLiteral(parser);

	if (!IS_TYPE(literal)) {
		error(parser, parser->previous, "Expected type after 'astype' keyword");
		freeLiteral(literal);
		return OP_EOF;
	}

	emitNodeLiteral(nodeHandle, literal);

	freeLiteral(literal);

	return OP_EOF;
}

static Opcode typeOf(Parser* parser, Node** nodeHandle) {
	Node* rhs = NULL;
	parsePrecedence(parser, &rhs, PREC_TERNARY);
	emitNodeUnary(nodeHandle, OP_TYPE_OF, rhs);
	return OP_EOF;
}

static Opcode compound(Parser* parser, Node** nodeHandle) {
	//read either an array or a dictionary into a literal node

	int iterations = 0; //count the number of entries iterated over

	//compound nodes to store what is read
	Node* array = NULL;
	Node* dictionary = NULL;

	while (!match(parser, TOKEN_BRACKET_RIGHT)) {
		//if empty dictionary, there will be a colon between the brackets
		if (iterations == 0 && match(parser, TOKEN_COLON)) {
			consume(parser, TOKEN_BRACKET_RIGHT, "Expected ']' at the end of empty dictionary definition");
			//emit an empty dictionary and finish
			emitNodeCompound(&dictionary, LITERAL_DICTIONARY);
			break;
		}

		if (iterations > 0) {
			consume(parser, TOKEN_COMMA, "Expected ',' in array or dictionary");
		}

		iterations++;

		Node* left = NULL;
		Node* right = NULL;

		//store the left
		parsePrecedence(parser, &left, PREC_PRIMARY);

		if (!left) { //error
			return OP_EOF;
		}

		//detect a dictionary
		if (match(parser, TOKEN_COLON)) {
			parsePrecedence(parser, &right, PREC_PRIMARY);

			if (!right) { //error
				freeNode(left);
				return OP_EOF;
			}

			//check we ARE defining a dictionary
			if (array) {
				error(parser, parser->previous, "Incorrect detection between array and dictionary");
				freeNode(array);
				return OP_EOF;
			}

			//init the dictionary
			if (!dictionary) {
				emitNodeCompound(&dictionary, LITERAL_DICTIONARY);
			}

			//grow the node if needed
			if (dictionary->compound.capacity < dictionary->compound.count + 1) {
				int oldCapacity = dictionary->compound.capacity;

				dictionary->compound.capacity = GROW_CAPACITY(oldCapacity);
				dictionary->compound.nodes = GROW_ARRAY(Node, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
			}

			//store the left and right in the node
			setNodePair(&dictionary->compound.nodes[dictionary->compound.count++], left, right);
		}
		//detect an array
		else {
			//check we ARE defining an array
			if (dictionary) {
				error(parser, parser->current, "Incorrect detection between array and dictionary");
				freeNode(dictionary);
				return OP_EOF;
			}

			//init the array
			if (!array) {
				emitNodeCompound(&array, LITERAL_ARRAY);
			}

			//grow the node if needed
			if (array->compound.capacity < array->compound.count + 1) {
				int oldCapacity = array->compound.capacity;

				array->compound.capacity = GROW_CAPACITY(oldCapacity);
				array->compound.nodes = GROW_ARRAY(Node, array->compound.nodes, oldCapacity, array->compound.capacity);
			}

			//copy into the array, and manually free the temp node
			array->compound.nodes[array->compound.count++] = *left;
			FREE(Node, left);
		}
	}

	//save the result
	if (array) {
		(*nodeHandle) = array;
	}
	else if (dictionary) {
		(*nodeHandle) = dictionary;
	}
	else {
		//both are null, must be an array (because reasons)
		emitNodeCompound(&array, LITERAL_ARRAY);
		(*nodeHandle) = array;
	}


	//ignored
	return OP_EOF;
}

static Opcode string(Parser* parser, Node** nodeHandle) {
	//handle strings
	switch(parser->previous.type) {
		case TOKEN_LITERAL_STRING: {
			int length = parser->previous.length;

			//for safety
			if (length > MAX_STRING_LENGTH) {
				length = MAX_STRING_LENGTH;
				char buffer[256];
				snprintf(buffer, 256, ERROR "Strings can only be a maximum of %d characters long" RESET, MAX_STRING_LENGTH);
				error(parser, parser->previous, buffer);
			}

			Literal literal = TO_STRING_LITERAL(copyString(parser->previous.lexeme, length), length);
			emitNodeLiteral(nodeHandle, literal);
			freeLiteral(literal);
			return OP_EOF;
		}

		//TODO: interpolated strings

		default:
			error(parser, parser->previous, "Unexpected token passed to string precedence rule");
			return OP_EOF;
	}
}

static Opcode grouping(Parser* parser, Node** nodeHandle) {
	//handle groupings with ()
	switch(parser->previous.type) {
		case TOKEN_PAREN_LEFT: {
			parsePrecedence(parser, nodeHandle, PREC_TERNARY);
			consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of grouping");

			//process the result without optimisations
			emitNodeGrouping(nodeHandle);
			return OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to grouping precedence rule");
			return OP_EOF;
	}
}

static Opcode binary(Parser* parser, Node** nodeHandle) {
	advance(parser);

	//binary() is an infix rule - so only get the RHS of the operator
	switch(parser->previous.type) {
		//arithmetic
		case TOKEN_PLUS: {
			parsePrecedence(parser, nodeHandle, PREC_TERM);
			return OP_ADDITION;
		}

		case TOKEN_MINUS: {
			parsePrecedence(parser, nodeHandle, PREC_TERM);
			return OP_SUBTRACTION;
		}

		case TOKEN_MULTIPLY: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return OP_MULTIPLICATION;
		}

		case TOKEN_DIVIDE: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return OP_DIVISION;
		}

		case TOKEN_MODULO: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return OP_MODULO;
		}

		//assignment
		case TOKEN_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_ASSIGN;
		}

		case TOKEN_PLUS_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_ADDITION_ASSIGN;
		}

		case TOKEN_MINUS_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_SUBTRACTION_ASSIGN;
		}

		case TOKEN_MULTIPLY_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_MULTIPLICATION_ASSIGN;
		}

		case TOKEN_DIVIDE_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_DIVISION_ASSIGN;
		}

		case TOKEN_MODULO_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_MODULO_ASSIGN;
		}

		//comparison
		case TOKEN_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_EQUAL;
		}

		case TOKEN_NOT_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_NOT_EQUAL;
		}

		case TOKEN_LESS: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_LESS;
		}

		case TOKEN_LESS_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_LESS_EQUAL;
		}

		case TOKEN_GREATER: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_GREATER;
		}

		case TOKEN_GREATER_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_COMPARE_GREATER_EQUAL;
		}

		case TOKEN_AND: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_AND;
		}

		case TOKEN_OR: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return OP_OR;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to binary precedence rule");
			return OP_EOF;
	}
}

static Opcode unary(Parser* parser, Node** nodeHandle) {
	Node* tmpNode = NULL;

	if (parser->previous.type == TOKEN_MINUS) {
		//temp handle to potentially negate values
		parsePrecedence(parser, &tmpNode, PREC_TERNARY); //can be a literal

		//check for negative literals (optimisation)
		if (tmpNode->type == NODE_LITERAL && (IS_INTEGER(tmpNode->atomic.literal) || IS_FLOAT(tmpNode->atomic.literal))) {
			//negate directly, if int or float
			Literal lit = tmpNode->atomic.literal;

			if (IS_INTEGER(lit)) {
				lit = TO_INTEGER_LITERAL(-AS_INTEGER(lit));
			}

			if (IS_FLOAT(lit)) {
				lit = TO_FLOAT_LITERAL(-AS_FLOAT(lit));
			}

			tmpNode->atomic.literal = lit;
			*nodeHandle = tmpNode;

			return OP_EOF;
		}

		//check for negated boolean errors
		if (tmpNode->type == NODE_LITERAL && IS_BOOLEAN(tmpNode->atomic.literal)) {
			error(parser, parser->previous, "Negative booleans are not allowed");
			return OP_EOF;
		}

		//actually emit the negation
		emitNodeUnary(nodeHandle, OP_NEGATE, tmpNode);
	}

	else if (parser->previous.type == TOKEN_NOT) {
		//temp handle to potentially negate values
		parsePrecedence(parser, &tmpNode, PREC_CALL); //can be a literal

		//check for inverted booleans
		if (tmpNode->type == NODE_LITERAL && IS_BOOLEAN(tmpNode->atomic.literal)) {
			//negate directly, if int or float
			Literal lit = tmpNode->atomic.literal;

			lit = TO_BOOLEAN_LITERAL(!AS_BOOLEAN(lit));

			tmpNode->atomic.literal = lit;
			*nodeHandle = tmpNode;

			return OP_EOF;
		}

		//check for inverted number errors
		if (tmpNode->type == NODE_LITERAL && (IS_INTEGER(tmpNode->atomic.literal) || IS_FLOAT(tmpNode->atomic.literal))) {
			error(parser, parser->previous, "Inverted numbers are not allowed");
			return OP_EOF;
		}

		//actually emit the negation
		emitNodeUnary(nodeHandle, OP_INVERT, tmpNode);
	}

	else {
		error(parser, parser->previous, "Unexpected token passed to unary precedence rule");
		return OP_EOF;
	}

	return OP_EOF;
}

static Opcode atomic(Parser* parser, Node** nodeHandle) {
	switch(parser->previous.type) {
		case TOKEN_NULL:
			emitNodeLiteral(nodeHandle, TO_NULL_LITERAL);
			return OP_EOF;

		case TOKEN_LITERAL_TRUE:
			emitNodeLiteral(nodeHandle, TO_BOOLEAN_LITERAL(true));
			return OP_EOF;

		case TOKEN_LITERAL_FALSE:
			emitNodeLiteral(nodeHandle, TO_BOOLEAN_LITERAL(false));
			return OP_EOF;

		case TOKEN_LITERAL_INTEGER: {
			int value = 0;
			sscanf(parser->previous.lexeme, "%d", &value);
			emitNodeLiteral(nodeHandle, TO_INTEGER_LITERAL(value));
			return OP_EOF;
		}

		case TOKEN_LITERAL_FLOAT: {
			float value = 0;
			sscanf(parser->previous.lexeme, "%f", &value);
			emitNodeLiteral(nodeHandle, TO_FLOAT_LITERAL(value));
			return OP_EOF;
		}

		case TOKEN_TYPE: {
			if (match(parser, TOKEN_CONST)) {
				emitNodeLiteral(nodeHandle, TO_TYPE_LITERAL(LITERAL_TYPE, true));
			}
			else {
				emitNodeLiteral(nodeHandle, TO_TYPE_LITERAL(LITERAL_TYPE, false));
			}

			return OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to atomic precedence rule");
			return OP_EOF;
	}
}

static Opcode identifier(Parser* parser, Node** nodeHandle) {
	//make a copy of the string
	Token identifierToken = parser->previous;

	if (identifierToken.type != TOKEN_IDENTIFIER) {
		error(parser, parser->previous, "Expected identifier");
		return OP_EOF;
	}

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	char* cpy = copyString(identifierToken.lexeme, length);
	Literal identifier = _toIdentifierLiteral(cpy, length); //BUGFIX: use this instead of the macro

	emitNodeLiteral(nodeHandle, identifier);

	freeLiteral(identifier); //don't leave it hanging

	return OP_EOF;
}

static Opcode castingPrefix(Parser* parser, Node** nodeHandle) {
	switch(parser->previous.type) {
		case TOKEN_BOOLEAN: {
			Literal literal = TO_TYPE_LITERAL(LITERAL_BOOLEAN, false);
			emitNodeLiteral(nodeHandle, literal);
			freeLiteral(literal);
		}
		break;

		case TOKEN_INTEGER: {
			Literal literal = TO_TYPE_LITERAL(LITERAL_INTEGER, false);
			emitNodeLiteral(nodeHandle, literal);
			freeLiteral(literal);
		}
		break;

		case TOKEN_FLOAT: {
			Literal literal = TO_TYPE_LITERAL(LITERAL_FLOAT, false);
			emitNodeLiteral(nodeHandle, literal);
			freeLiteral(literal);
		}
		break;

		case TOKEN_STRING: {
			Literal literal = TO_TYPE_LITERAL(LITERAL_STRING, false);
			emitNodeLiteral(nodeHandle, literal);
			freeLiteral(literal);
		}
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to casting precedence rule");
			return OP_EOF;
	}

	return OP_EOF;
}

static Opcode castingInfix(Parser* parser, Node** nodeHandle) {
	advance(parser);

	switch(parser->previous.type) {
		case TOKEN_IDENTIFIER:
			identifier(parser, nodeHandle);
		break;

		case TOKEN_LITERAL_TRUE:
		case TOKEN_LITERAL_FALSE:
			atomic(parser, nodeHandle);
		break;

		case TOKEN_LITERAL_INTEGER:
			atomic(parser, nodeHandle);
		break;

		case TOKEN_LITERAL_FLOAT:
			atomic(parser, nodeHandle);
		break;

		case TOKEN_LITERAL_STRING:
			atomic(parser, nodeHandle);
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to casting infix precedence rule");
			return OP_EOF;
	}

	return OP_TYPE_CAST;
}

static Opcode incrementPrefix(Parser* parser, Node** nodeHandle) {
	advance(parser);

	Node* node = NULL;
	identifier(parser, &node);

	emitNodePrefixIncrement(nodeHandle, node->atomic.literal, 1);

	freeNode(node);

	return OP_EOF;
}

static Opcode incrementInfix(Parser* parser, Node** nodeHandle) {
	Node* node = NULL;
	identifier(parser, &node);

	advance(parser);

	emitNodePostfixIncrement(nodeHandle, node->atomic.literal, 1);

	freeNode(node);

	return OP_EOF;
}

static Opcode decrementPrefix(Parser* parser, Node** nodeHandle) {
	advance(parser);

	Node* node = NULL;
	identifier(parser, &node); //weird

	emitNodePrefixIncrement(nodeHandle, node->atomic.literal, -1);

	freeNode(node);

	return OP_EOF;
}

static Opcode decrementInfix(Parser* parser, Node** nodeHandle) {
	Node* node = NULL;
	identifier(parser, &node);

	advance(parser);

	emitNodePostfixIncrement(nodeHandle, node->atomic.literal, -1);

	freeNode(node);

	return OP_EOF;
}

static Opcode fnCall(Parser* parser, Node** nodeHandle) {
	advance(parser); //skip the left paren

	//binary() is an infix rule - so only get the RHS of the operator
	switch(parser->previous.type) {
		//arithmetic
		case TOKEN_PAREN_LEFT: {
			Node* arguments = NULL;
			emitNodeFnCollection(&arguments);

			//if there's arguments
			if (!match(parser, TOKEN_PAREN_RIGHT)) {
				//read each argument
				do {
					//emit the node to the argument list (grow the node if needed)
					if (arguments->fnCollection.capacity < arguments->fnCollection.count + 1) {
						int oldCapacity = arguments->fnCollection.capacity;

						arguments->fnCollection.capacity = GROW_CAPACITY(oldCapacity);
						arguments->fnCollection.nodes = GROW_ARRAY(Node, arguments->fnCollection.nodes, oldCapacity, arguments->fnCollection.capacity);
					}

					Node* node = NULL;
					parsePrecedence(parser, &node, PREC_TERNARY);
					arguments->fnCollection.nodes[arguments->fnCollection.count++] = *node;
					FREE(Node, node);
				} while(match(parser, TOKEN_COMMA));

				consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of argument list");
			}

			//emit the call
			emitFnCall(nodeHandle, arguments, arguments->fnCollection.count);

			return OP_FN_CALL;
		}
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to function call precedence rule");
			return OP_EOF;
	}

	return OP_EOF;
}

static Opcode indexAccess(Parser* parser, Node** nodeHandle) {
	advance(parser);

	//val[first : second : third]

	Node* first = NULL;
	Node* second = NULL;
	Node* third = NULL;

	//booleans indicate blank slice indexing
	emitNodeLiteral(&first, TO_BOOLEAN_LITERAL(true));
	emitNodeLiteral(&second, TO_BOOLEAN_LITERAL(true));
	emitNodeLiteral(&third, TO_BOOLEAN_LITERAL(true));

	bool readFirst = false; //pattern matching is bullcrap

	//eat the first
	if (!match(parser, TOKEN_COLON)) {
		freeNode(first);
		parsePrecedence(parser, &first, PREC_TERNARY);
		match(parser, TOKEN_COLON);
		readFirst = true;
	}

	if (match(parser, TOKEN_BRACKET_RIGHT)) {

		if (readFirst) {
			freeNode(second);
			second = NULL;
		}

		freeNode(third);
		third = NULL;

		emitNodeIndex(nodeHandle, first, second, third);
		return OP_INDEX;
	}

	//eat the second
	if (!match(parser, TOKEN_COLON)) {
		freeNode(second);
		parsePrecedence(parser, &second, PREC_TERNARY);
		match(parser, TOKEN_COLON);
	}

	if (match(parser, TOKEN_BRACKET_RIGHT)) {
		freeNode(third);
		third = NULL;
		emitNodeIndex(nodeHandle, first, second, third);
		return OP_INDEX;
	}

	//eat the third
	freeNode(third);
	parsePrecedence(parser, &third, PREC_TERNARY);
	emitNodeIndex(nodeHandle, first, second, third);

	consume(parser, TOKEN_BRACKET_RIGHT, "Expected ']' in index notation");

	return OP_INDEX;
}

static Opcode dot(Parser* parser, Node** nodeHandle) {
	advance(parser); //for the dot

	Node* node = NULL;

	parsePrecedence(parser, &node, PREC_CALL);

	if (node == NULL || node->binary.right == NULL) {
		error(parser, parser->previous, "Expected function call after dot operator");
		return OP_EOF;
	}

	//hijack the function call, and hack in an extra parameter
	node->binary.right->fnCall.argumentCount++;
	node->binary.opcode = OP_DOT;

	(*nodeHandle) = node;
	return OP_DOT; //signal that the function name and arguments are in the wrong order
}

ParseRule parseRules[] = { //must match the token types
	//types
	{atomic, NULL, PREC_PRIMARY},// TOKEN_NULL,
	{castingPrefix, NULL, PREC_CALL},// TOKEN_BOOLEAN,
	{castingPrefix, NULL, PREC_CALL},// TOKEN_INTEGER,
	{castingPrefix, NULL, PREC_CALL},// TOKEN_FLOAT,
	{castingPrefix, NULL, PREC_CALL},// TOKEN_STRING,
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
	{atomic, NULL, PREC_NONE},// TOKEN_TYPE,
	{asType, NULL, PREC_PRIMARY},// TOKEN_ASTYPE,
	{typeOf, NULL, PREC_CALL},// TOKEN_TYPEOF,
	{NULL, NULL, PREC_NONE},// TOKEN_VAR,
	{NULL, NULL, PREC_NONE},// TOKEN_WHILE,

	//literal values
	{identifier, castingInfix, PREC_PRIMARY},// TOKEN_IDENTIFIER,
	{atomic, castingInfix, PREC_PRIMARY},// TOKEN_LITERAL_TRUE,
	{atomic, castingInfix, PREC_PRIMARY},// TOKEN_LITERAL_FALSE,
	{atomic, castingInfix, PREC_PRIMARY},// TOKEN_LITERAL_INTEGER,
	{atomic, castingInfix, PREC_PRIMARY},// TOKEN_LITERAL_FLOAT,
	{string, castingInfix, PREC_PRIMARY},// TOKEN_LITERAL_STRING,

	//math operators
	{NULL, binary, PREC_TERM},// TOKEN_PLUS,
	{unary, binary, PREC_TERM},// TOKEN_MINUS,
	{NULL, binary, PREC_FACTOR},// TOKEN_MULTIPLY,
	{NULL, binary, PREC_FACTOR},// TOKEN_DIVIDE,
	{NULL, binary, PREC_FACTOR},// TOKEN_MODULO,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_PLUS_ASSIGN,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_MINUS_ASSIGN,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_MULTIPLY_ASSIGN,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_DIVIDE_ASSIGN,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_MODULO_ASSIGN,
	{incrementPrefix, incrementInfix, PREC_CALL},// TOKEN_PLUS_PLUS,
	{decrementPrefix, decrementInfix, PREC_CALL},// TOKEN_MINUS_MINUS,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_ASSIGN,

	//logical operators
	{grouping, fnCall, PREC_CALL},// TOKEN_PAREN_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_PAREN_RIGHT,
	{compound, indexAccess, PREC_CALL},// TOKEN_BRACKET_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACKET_RIGHT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACE_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_BRACE_RIGHT,
	{unary, NULL, PREC_CALL},// TOKEN_NOT,
	{NULL, binary, PREC_COMPARISON},// TOKEN_NOT_EQUAL,
	{NULL, binary, PREC_COMPARISON},// TOKEN_EQUAL,
	{NULL, binary, PREC_COMPARISON},// TOKEN_LESS,
	{NULL, binary, PREC_COMPARISON},// TOKEN_GREATER,
	{NULL, binary, PREC_COMPARISON},// TOKEN_LESS_EQUAL,
	{NULL, binary, PREC_COMPARISON},// TOKEN_GREATER_EQUAL,
	{NULL, binary, PREC_AND},// TOKEN_AND,
	{NULL, binary, PREC_OR},// TOKEN_OR,

	//other operators
	{NULL, NULL, PREC_NONE},// TOKEN_COLON,
	{NULL, NULL, PREC_NONE},// TOKEN_SEMICOLON,
	{NULL, NULL, PREC_NONE},// TOKEN_COMMA,
	{NULL, dot, PREC_CALL},// TOKEN_DOT,
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

//constant folding
static bool calcStaticBinaryArithmetic(Parser* parser, Node** nodeHandle) {
	switch((*nodeHandle)->binary.opcode) {
		case OP_ADDITION:
		case OP_SUBTRACTION:
		case OP_MULTIPLICATION:
		case OP_DIVISION:
		case OP_MODULO:
		case OP_COMPARE_EQUAL:
		case OP_COMPARE_NOT_EQUAL:
		case OP_COMPARE_LESS:
		case OP_COMPARE_LESS_EQUAL:
		case OP_COMPARE_GREATER:
		case OP_COMPARE_GREATER_EQUAL:
			break;

		default:
			return true;
	}

	//recurse to the left and right
	if ((*nodeHandle)->binary.left->type == NODE_BINARY) {
		calcStaticBinaryArithmetic(parser, &(*nodeHandle)->binary.left);
	}

	if ((*nodeHandle)->binary.right->type == NODE_BINARY) {
		calcStaticBinaryArithmetic(parser, &(*nodeHandle)->binary.right);
	}

	//make sure left and right are both literals
	if (!((*nodeHandle)->binary.left->type == NODE_LITERAL && (*nodeHandle)->binary.right->type == NODE_LITERAL)) {
		return true;
	}

	//evaluate
	Literal lhs = (*nodeHandle)->binary.left->atomic.literal;
	Literal rhs = (*nodeHandle)->binary.right->atomic.literal;
	Literal result = TO_NULL_LITERAL;

	//type coersion
	if (IS_FLOAT(lhs) && IS_INTEGER(rhs)) {
		rhs = TO_FLOAT_LITERAL(AS_INTEGER(rhs));
	}

	if (IS_INTEGER(lhs) && IS_FLOAT(rhs)) {
		lhs = TO_FLOAT_LITERAL(AS_INTEGER(lhs));
	}

	//maths based on types
	if(IS_INTEGER(lhs) && IS_INTEGER(rhs)) {
		switch((*nodeHandle)->binary.opcode) {
			case OP_ADDITION:
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) + AS_INTEGER(rhs) );
			break;

			case OP_SUBTRACTION:
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) - AS_INTEGER(rhs) );
			break;

			case OP_MULTIPLICATION:
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) * AS_INTEGER(rhs) );
			break;

			case OP_DIVISION:
				if (AS_INTEGER(rhs) == 0) {
					error(parser, parser->previous, "Can't divide by zero (error found in constant folding)");
					return false;
				}
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) / AS_INTEGER(rhs) );
			break;

			case OP_MODULO:
				if (AS_INTEGER(rhs) == 0) {
					error(parser, parser->previous, "Can't modulo by zero (error found in constant folding)");
					return false;
				}
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) % AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) == AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_NOT_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) != AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_LESS:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) < AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_LESS_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) <= AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_GREATER:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) > AS_INTEGER(rhs) );
			break;

			case OP_COMPARE_GREATER_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_INTEGER(lhs) >= AS_INTEGER(rhs) );
			break;

			default:
				error(parser, parser->previous, "[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
				return false;
		}
	}

	//catch bad modulo
	if ((IS_FLOAT(lhs) || IS_FLOAT(rhs)) && (*nodeHandle)->binary.opcode == OP_MODULO) {
		error(parser, parser->previous, "Bad arithmetic argument (modulo on floats not allowed)");
		return false;
	}

	if(IS_FLOAT(lhs) && IS_FLOAT(rhs)) {
		switch((*nodeHandle)->binary.opcode) {
			case OP_ADDITION:
				result = TO_FLOAT_LITERAL( AS_FLOAT(lhs) + AS_FLOAT(rhs) );
			break;

			case OP_SUBTRACTION:
				result = TO_FLOAT_LITERAL( AS_FLOAT(lhs) - AS_FLOAT(rhs) );
			break;

			case OP_MULTIPLICATION:
				result = TO_FLOAT_LITERAL( AS_FLOAT(lhs) * AS_FLOAT(rhs) );
			break;

			case OP_DIVISION:
				if (AS_FLOAT(rhs) == 0) {
					error(parser, parser->previous, "Can't divide by zero (error found in constant folding)");
					return false;
				}
				result = TO_FLOAT_LITERAL( AS_FLOAT(lhs) / AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) == AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_NOT_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) != AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_LESS:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) < AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_LESS_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) <= AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_GREATER:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) > AS_FLOAT(rhs) );
			break;

			case OP_COMPARE_GREATER_EQUAL:
				result = TO_BOOLEAN_LITERAL( AS_FLOAT(lhs) >= AS_FLOAT(rhs) );
			break;

			default:
				error(parser, parser->previous, "[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
				return false;
		}
	}

	//nothing can be done to optimize
	if (IS_NULL(result)) {
		return true;
	}

	//optimize by converting this node into a literal node
	freeNode((*nodeHandle)->binary.left);
	freeNode((*nodeHandle)->binary.right);

	(*nodeHandle)->type = NODE_LITERAL;
	(*nodeHandle)->atomic.literal = result;

	return true;
}

static void parsePrecedence(Parser* parser, Node** nodeHandle, PrecedenceRule rule) {
	//every valid expression has a prefix rule
	advance(parser);
	ParseFn prefixRule = getRule(parser->previous.type)->prefix;

	if (prefixRule == NULL) {
		*nodeHandle = NULL; //the handle's value MUST be set to null for error handling
		error(parser, parser->previous, "Expected expression");
		return;
	}

	bool canBeAssigned = rule <= PREC_ASSIGNMENT;
	prefixRule(parser, nodeHandle); //ignore the returned opcode

	//infix rules are left-recursive
	while (rule <= getRule(parser->current.type)->precedence) {
		ParseFn infixRule = getRule(parser->current.type)->infix;

		if (infixRule == NULL) {
			*nodeHandle = NULL; //the handle's value MUST be set to null for error handling
			error(parser, parser->current, "Expected operator");
			return;
		}

		Node* rhsNode = NULL;
		const Opcode opcode = infixRule(parser, &rhsNode); //NOTE: infix rule must advance the parser

		if (opcode == OP_EOF) {
			freeNode(*nodeHandle);
			*nodeHandle = rhsNode;
			return; //we're done here
		}

		emitNodeBinary(nodeHandle, rhsNode, opcode);

		if (!calcStaticBinaryArithmetic(parser, nodeHandle)) {
			return;
		}
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
static void blockStmt(Parser* parser, Node** nodeHandle) {
	//init
	emitNodeBlock(nodeHandle);

	//sub-scope, compile it and push it up in a node
	while (!match(parser, TOKEN_BRACE_RIGHT)) {
		if ((*nodeHandle)->block.capacity < (*nodeHandle)->block.count + 1) {
			int oldCapacity = (*nodeHandle)->block.capacity;

			(*nodeHandle)->block.capacity = GROW_CAPACITY(oldCapacity);
			(*nodeHandle)->block.nodes = GROW_ARRAY(Node, (*nodeHandle)->block.nodes, oldCapacity, (*nodeHandle)->block.capacity);
		}

		Node* node = NULL;

		//process the grammar rule for this line
		declaration(parser, &node);

		// Ground floor: perfumery / Stationery and leather goods / Wigs and haberdashery / Kitchenware and food / Going up!
		if (parser->panic) {
			return;
		}

		//BUGFIX: statements no longer require an existing node
		((*nodeHandle)->block.nodes[(*nodeHandle)->block.count++]) = *node;
		FREE(Node, node); //simply free the tmp node
	}
}

static void printStmt(Parser* parser, Node** nodeHandle) {
	//set the node info
	Node* node = NULL;
	expression(parser, &node);
	emitNodeUnary(nodeHandle, OP_PRINT, node);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of print statement");
}

static void assertStmt(Parser* parser, Node** nodeHandle) {
	//set the node info
	(*nodeHandle) = ALLOCATE(Node, 1); //special case, because I'm lazy
	(*nodeHandle)->type = NODE_BINARY;
	(*nodeHandle)->binary.opcode = OP_ASSERT;

	parsePrecedence(parser, &((*nodeHandle)->binary.left), PREC_TERNARY);
	consume(parser, TOKEN_COMMA, "Expected ',' in assert statement");
	parsePrecedence(parser, &((*nodeHandle)->binary.right), PREC_TERNARY);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of assert statement");
}

static void ifStmt(Parser* parser, Node** nodeHandle) {
	Node* condition = NULL;
	Node* thenPath = NULL;
	Node* elsePath = NULL;

	//read the condition
	consume(parser, TOKEN_PAREN_LEFT, "Expected '(' at beginning of if clause");
	parsePrecedence(parser, &condition, PREC_TERNARY);

	//read the then path
	consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of if clause");
	declaration(parser, &thenPath);

	//read the optional else path
	if (match(parser, TOKEN_ELSE)) {
		declaration(parser, &elsePath);
	}

	emitNodePath(nodeHandle, NODE_PATH_IF, NULL, NULL, condition, thenPath, elsePath);
}

static void whileStmt(Parser* parser, Node** nodeHandle) {
	Node* condition = NULL;
	Node* thenPath = NULL;

	//read the condition
	consume(parser, TOKEN_PAREN_LEFT, "Expected '(' at beginning of while clause");
	parsePrecedence(parser, &condition, PREC_TERNARY);

	//read the then path
	consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of while clause");
	declaration(parser, &thenPath);

	emitNodePath(nodeHandle, NODE_PATH_WHILE, NULL, NULL, condition, thenPath, NULL);
}

static void forStmt(Parser* parser, Node** nodeHandle) {
	Node* preClause = NULL;
	Node* postClause = NULL;
	Node* condition = NULL;
	Node* thenPath = NULL;

	//read the clauses
	consume(parser, TOKEN_PAREN_LEFT, "Expected '(' at beginning of for clause");
	declaration(parser, &preClause);

	parsePrecedence(parser, &condition, PREC_TERNARY);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' after condition of for clause");
	parsePrecedence(parser, &postClause, PREC_ASSIGNMENT);

	consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of for clause");

	//read the path
	declaration(parser, &thenPath);

	emitNodePath(nodeHandle, NODE_PATH_FOR, preClause, postClause, condition, thenPath, NULL);
}

static void breakStmt(Parser* parser, Node** nodeHandle) {
	emitNodePath(nodeHandle, NODE_PATH_BREAK, NULL, NULL, NULL, NULL, NULL);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of break statement");
}

static void continueStmt(Parser* parser, Node** nodeHandle) {
	emitNodePath(nodeHandle, NODE_PATH_CONTINUE, NULL, NULL, NULL, NULL, NULL);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of continue statement");
}

static void returnStmt(Parser* parser, Node** nodeHandle) {
	Node* returnValues = NULL;
	emitNodeFnCollection(&returnValues);

	if (!match(parser, TOKEN_SEMICOLON)) {
		do { //loop for multiple returns (disabled later in the pipeline)
			//append the node to the return list (grow the node if needed)
			if (returnValues->fnCollection.capacity < returnValues->fnCollection.count + 1) {
				int oldCapacity = returnValues->fnCollection.capacity;

				returnValues->fnCollection.capacity = GROW_CAPACITY(oldCapacity);
				returnValues->fnCollection.nodes = GROW_ARRAY(Node, returnValues->fnCollection.nodes, oldCapacity, returnValues->fnCollection.capacity);
			}

			Node* node = NULL;
			parsePrecedence(parser, &node, PREC_TERNARY);

			returnValues->fnCollection.nodes[returnValues->fnCollection.count++] = *node;
			FREE(Node, node);
		} while(match(parser, TOKEN_COMMA));

		consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of return statement");
	}

	emitNodePath(nodeHandle, NODE_PATH_RETURN, NULL, NULL, NULL, returnValues, NULL);
}

static void importStmt(Parser* parser, Node** nodeHandle) {
	//read the identifier
	Node* node = NULL;
	advance(parser);
	identifier(parser, &node);

	if (node == NULL) {
		return;
	}

	Literal idn = copyLiteral(node->atomic.literal);
	freeNode(node);

	Literal alias = TO_NULL_LITERAL;

	if (match(parser, TOKEN_AS)) {
		advance(parser);
		identifier(parser, &node);
		alias = copyLiteral(node->atomic.literal);
		freeNode(node);
	}

	emitNodeImport(nodeHandle, NODE_IMPORT, idn, alias);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of import statement");

	freeLiteral(idn);
	freeLiteral(alias);
}

static void exportStmt(Parser* parser, Node** nodeHandle) {
	//read the identifier
	Node* node = NULL;
	advance(parser);
	identifier(parser, &node);

	if (node == NULL) {
		return;
	}

	Literal idn = copyLiteral(node->atomic.literal);
	freeNode(node);

	Literal alias = TO_NULL_LITERAL;

	if (match(parser, TOKEN_AS)) {
		advance(parser);
		identifier(parser, &node);
		alias = copyLiteral(node->atomic.literal);
		freeNode(node);
	}

	emitNodeImport(nodeHandle, NODE_EXPORT, idn, alias);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of export statement");

	freeLiteral(idn);
	freeLiteral(alias);
}

//precedence functions
static void expressionStmt(Parser* parser, Node** nodeHandle) {
	//BUGFIX: check for empty statements
	if (match(parser, TOKEN_SEMICOLON)) {
		emitNodeLiteral(nodeHandle, TO_NULL_LITERAL);
		return;
	}

	Node* ptr = NULL;
	expression(parser, &ptr);

	if (ptr != NULL) {
		*nodeHandle = ptr;
	}

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at the end of expression statement");
}

static void statement(Parser* parser, Node** nodeHandle) {
	//block
	if (match(parser, TOKEN_BRACE_LEFT)) {
		blockStmt(parser, nodeHandle);
		return;
	}

	//print
	if (match(parser, TOKEN_PRINT)) {
		printStmt(parser, nodeHandle);
		return;
	}

	//assert
	if (match(parser, TOKEN_ASSERT)) {
		assertStmt(parser, nodeHandle);
		return;
	}

	//if-then-else
	if (match(parser, TOKEN_IF)) {
		ifStmt(parser, nodeHandle);
		return;
	}

	//while-then
	if (match(parser, TOKEN_WHILE)) {
		whileStmt(parser, nodeHandle);
		return;
	}

	//for-pre-clause-post-then
	if (match(parser, TOKEN_FOR)) {
		forStmt(parser, nodeHandle);
		return;
	}

	//break
	if (match(parser, TOKEN_BREAK)) {
		breakStmt(parser, nodeHandle);
		return;
	}

	//continue
	if (match(parser, TOKEN_CONTINUE)) {
		continueStmt(parser, nodeHandle);
		return;
	}

	//return
	if (match(parser, TOKEN_RETURN)) {
		returnStmt(parser, nodeHandle);
		return;
	}

	//import
	if (match(parser, TOKEN_IMPORT)) {
		importStmt(parser, nodeHandle);
		return;
	}

	//export
	if (match(parser, TOKEN_EXPORT)) {
		exportStmt(parser, nodeHandle);
		return;
	}

	//default
	expressionStmt(parser, nodeHandle);
}

//declarations and definitions
static Literal readTypeToLiteral(Parser* parser) {
	advance(parser);

	Literal literal = TO_TYPE_LITERAL(LITERAL_NULL, false);

	switch(parser->previous.type) {
		case TOKEN_BOOLEAN:
			AS_TYPE(literal).typeOf = LITERAL_BOOLEAN;
		break;

		case TOKEN_INTEGER:
			AS_TYPE(literal).typeOf = LITERAL_INTEGER;
		break;

		case TOKEN_FLOAT:
			AS_TYPE(literal).typeOf = LITERAL_FLOAT;
		break;

		case TOKEN_STRING:
			AS_TYPE(literal).typeOf = LITERAL_STRING;
		break;

		//array, dictionary - read the sub-types
		case TOKEN_BRACKET_LEFT: {
			Literal l = readTypeToLiteral(parser);

			if (match(parser, TOKEN_COLON)) {
				Literal r = readTypeToLiteral(parser);

				TYPE_PUSH_SUBTYPE(&literal, l);
				TYPE_PUSH_SUBTYPE(&literal, r);

				AS_TYPE(literal).typeOf = LITERAL_DICTIONARY;
			}
			else {
				TYPE_PUSH_SUBTYPE(&literal, l);

				AS_TYPE(literal).typeOf = LITERAL_ARRAY;
			}

			consume(parser, TOKEN_BRACKET_RIGHT, "Expected ']' at end of type definition");
		}
		break;

		case TOKEN_FUNCTION:
			AS_TYPE(literal).typeOf = LITERAL_FUNCTION;
		break;

		case TOKEN_ANY:
			AS_TYPE(literal).typeOf = LITERAL_ANY;
		break;

		//wtf
		case TOKEN_IDENTIFIER: {
			//duplicated from identifier()
			Token identifierToken = parser->previous;
			int length = identifierToken.length;
			//for safety
			if (length > 256) {
				length = 256;
				error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
			}
			char* cpy = copyString(identifierToken.lexeme, length);
			literal = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro
		}
		break;

		//WTF
		case TOKEN_TYPE:
			AS_TYPE(literal).typeOf = LITERAL_TYPE;
		break;

		default:
			error(parser, parser->previous, "Bad type signature");
			return TO_NULL_LITERAL;
	}

	//const follows the type
	if (match(parser, TOKEN_CONST)) {
		AS_TYPE(literal).constant = true;
	}

	return literal;
}

static void varDecl(Parser* parser, Node** nodeHandle) {
	//read the identifier
	consume(parser, TOKEN_IDENTIFIER, "Expected identifier after var keyword");
	Token identifierToken = parser->previous;

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	char* cpy = copyString(identifierToken.lexeme, length);
	Literal identifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

	//read the type, if present
	Literal typeLiteral;
	if (match(parser, TOKEN_COLON)) {
		typeLiteral = readTypeToLiteral(parser);
	}
	else {
		//default to non-const any
		typeLiteral = TO_TYPE_LITERAL(LITERAL_ANY, false);
	}

	//variable definition is an expression
	Node* expressionNode = NULL;
	if (match(parser, TOKEN_ASSIGN)) {
		expression(parser, &expressionNode);
	}
	else {
		//values are null by default
		emitNodeLiteral(&expressionNode, TO_NULL_LITERAL);
	}

	//TODO: static type checking?

	//declare it
	emitNodeVarDecl(nodeHandle, identifier, typeLiteral, expressionNode);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of var declaration");
}

static void fnDecl(Parser* parser, Node** nodeHandle) {
	//read the identifier
	consume(parser, TOKEN_IDENTIFIER, "Expected identifier after fn keyword");
	Token identifierToken = parser->previous;

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	char* cpy = copyString(identifierToken.lexeme, length);
	Literal identifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

	//read the parameters and arity
	consume(parser, TOKEN_PAREN_LEFT, "Expected '(' after function identifier");

	//for holding the array of arguments
	Node* argumentNode = NULL;
	emitNodeFnCollection(&argumentNode);

	//read args
	if (!match(parser, TOKEN_PAREN_RIGHT)) {
		do {
			//check for rest parameter
			if (match(parser, TOKEN_REST)) {
				//read the argument identifier
				consume(parser, TOKEN_IDENTIFIER, "Expected identifier as function argument");
				Token argIdentifierToken = parser->previous;

				int length = argIdentifierToken.length;

				//for safety
				if (length > 256) {
					length = 256;
					error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
				}

				char* cpy = copyString(argIdentifierToken.lexeme, length);
				Literal argIdentifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

				//set the type (array of any types)
				Literal argTypeLiteral = TO_TYPE_LITERAL(LITERAL_FUNCTION_ARG_REST, false);

				//emit the node to the argument list (grow the node if needed)
				if (argumentNode->fnCollection.capacity < argumentNode->fnCollection.count + 1) {
					int oldCapacity = argumentNode->fnCollection.capacity;

					argumentNode->fnCollection.capacity = GROW_CAPACITY(oldCapacity);
					argumentNode->fnCollection.nodes = GROW_ARRAY(Node, argumentNode->fnCollection.nodes, oldCapacity, argumentNode->fnCollection.capacity);
				}

				//store the arg in the array
				Node* literalNode = NULL;
				emitNodeVarDecl(&literalNode, argIdentifier, argTypeLiteral, NULL);

				argumentNode->fnCollection.nodes[argumentNode->fnCollection.count++] = *literalNode;
				FREE(Node, literalNode);

				break;
			}

			//read the argument identifier
			consume(parser, TOKEN_IDENTIFIER, "Expected identifier as function argument");
			Token argIdentifierToken = parser->previous;

			int length = argIdentifierToken.length;

			//for safety
			if (length > 256) {
				length = 256;
				error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
			}

			char* cpy = copyString(argIdentifierToken.lexeme, length);

			Literal argIdentifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

			//read optional type of the identifier
			Literal argTypeLiteral;
			if (match(parser, TOKEN_COLON)) {
				argTypeLiteral = readTypeToLiteral(parser);
			}
			else {
				//default to non-const any
				argTypeLiteral = TO_TYPE_LITERAL(LITERAL_ANY, false);
			}

			//emit the node to the argument list (grow the node if needed)
			if (argumentNode->fnCollection.capacity < argumentNode->fnCollection.count + 1) {
				int oldCapacity = argumentNode->fnCollection.capacity;

				argumentNode->fnCollection.capacity = GROW_CAPACITY(oldCapacity);
				argumentNode->fnCollection.nodes = GROW_ARRAY(Node, argumentNode->fnCollection.nodes, oldCapacity, argumentNode->fnCollection.capacity);
			}

			//store the arg in the array
			Node* literalNode = NULL;
			emitNodeVarDecl(&literalNode, argIdentifier, argTypeLiteral, NULL);

			argumentNode->fnCollection.nodes[argumentNode->fnCollection.count++] = *literalNode;
			FREE(Node, literalNode);

		} while (match(parser, TOKEN_COMMA)); //if comma is read, continue

		consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' after function argument list");
	}

	//read the return types, if present
	Node* returnNode = NULL;
	emitNodeFnCollection(&returnNode);

	if (match(parser, TOKEN_COLON)) {
		do {
			//append the node to the return list (grow the node if needed)
			if (returnNode->fnCollection.capacity < returnNode->fnCollection.count + 1) {
				int oldCapacity = returnNode->fnCollection.capacity;

				returnNode->fnCollection.capacity = GROW_CAPACITY(oldCapacity);
				returnNode->fnCollection.nodes = GROW_ARRAY(Node, returnNode->fnCollection.nodes, oldCapacity, returnNode->fnCollection.capacity);
			}

			Node* literalNode = NULL;
			emitNodeLiteral(&literalNode, readTypeToLiteral(parser));

			returnNode->fnCollection.nodes[returnNode->fnCollection.count++] = *literalNode;
			FREE(Node, literalNode);
		} while(match(parser, TOKEN_COMMA));
	}

	//read the function body
	consume(parser, TOKEN_BRACE_LEFT, "Expected '{' after return list");

	Node* blockNode = NULL;
	blockStmt(parser, &blockNode);

	//declare it
	emitNodeFnDecl(nodeHandle, identifier, argumentNode, returnNode, blockNode);
}

static void declaration(Parser* parser, Node** nodeHandle) { //assume nodeHandle holds a blank node
	//variable declarations
	if (match(parser, TOKEN_VAR)) {
		varDecl(parser, nodeHandle);
	}
	else if (match(parser, TOKEN_FUNCTION)) {
		fnDecl(parser, nodeHandle);
	}
	else {
		statement(parser, nodeHandle);
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
	parser->lexer = NULL;
	parser->error = false;
	parser->panic = false;

	parser->previous.type = TOKEN_NULL;
	parser->current.type = TOKEN_NULL;
}

Node* scanParser(Parser* parser) {
	//check for EOF
	if (match(parser, TOKEN_EOF)) {
		return NULL;
	}

	//returns nodes on the heap
	Node* node = NULL;

	//process the grammar rule for this line
	declaration(parser, &node);

	if (parser->panic) {
		synchronize(parser);
		//return an error node for this iteration
		freeNode(node);
		node = ALLOCATE(Node, 1);
		node->type = NODE_ERROR;
	}

	return node;
}

