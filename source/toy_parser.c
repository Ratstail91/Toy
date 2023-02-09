#include "toy_parser.h"

#include "toy_memory.h"
#include "toy_literal.h"
#include "toy_opcodes.h"

#include "toy_console_colors.h"

#include <stdio.h>

//utility functions
static void error(Toy_Parser* parser, Toy_Token token, const char* message) {
	//keep going while panicing
	if (parser->panic) return;

	fprintf(stderr, TOY_CC_ERROR "[Line %d] Error", token.line);

	//check type
	if (token.type == TOY_TOKEN_EOF) {
		fprintf(stderr, " at end");
	}

	else {
		fprintf(stderr, " at '%.*s'", token.length, token.lexeme);
	}

	//finally
	fprintf(stderr, ": %s\n" TOY_CC_RESET, message);
	parser->error = true;
	parser->panic = true;
}

static void advance(Toy_Parser* parser) {
	parser->previous = parser->current;
	parser->current = Toy_scanLexer(parser->lexer);

	if (parser->current.type == TOY_TOKEN_ERROR) {
		error(parser, parser->current, "Toy_Lexer error");
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
		error(parser, parser->current, msg);
		return;
	}

	advance(parser);
}

static void synchronize(Toy_Parser* parser) {
#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		fprintf(stderr, TOY_CC_ERROR "Synchronizing input\n" TOY_CC_RESET);
	}
#endif

	while (parser->current.type != TOY_TOKEN_EOF) {
		switch(parser->current.type) {
			//these tokens can start a line
			case TOY_TOKEN_ASSERT:
			case TOY_TOKEN_BREAK:
			case TOY_TOKEN_CLASS:
			case TOY_TOKEN_CONTINUE:
			case TOY_TOKEN_DO:
			case TOY_TOKEN_EXPORT:
			case TOY_TOKEN_FOR:
			case TOY_TOKEN_FOREACH:
			case TOY_TOKEN_IF:
			case TOY_TOKEN_IMPORT:
			case TOY_TOKEN_PRINT:
			case TOY_TOKEN_RETURN:
			case TOY_TOKEN_VAR:
			case TOY_TOKEN_WHILE:
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

typedef Toy_Opcode (*ParseFn)(Toy_Parser* parser, Toy_ASTNode** nodeHandle);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	PrecedenceRule precedence;
} ParseRule;

//no static!
ParseRule parseRules[];

//forward declarations
static void declaration(Toy_Parser* parser, Toy_ASTNode** nodeHandle);
static void parsePrecedence(Toy_Parser* parser, Toy_ASTNode** nodeHandle, PrecedenceRule rule);
static Toy_Literal readTypeToLiteral(Toy_Parser* parser);

//TODO: resolve the messy order of these
//the expression rules
static Toy_Opcode asType(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_Literal literal = readTypeToLiteral(parser);

	if (!TOY_IS_TYPE(literal)) {
		error(parser, parser->previous, "Expected type after 'astype' keyword");
		Toy_freeLiteral(literal);
		return TOY_OP_EOF;
	}

	Toy_emitASTNodeLiteral(nodeHandle, literal);

	Toy_freeLiteral(literal);

	return TOY_OP_EOF;
}

static Toy_Opcode typeOf(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* rhs = NULL;
	parsePrecedence(parser, &rhs, PREC_TERNARY);
	Toy_emitASTNodeUnary(nodeHandle, TOY_OP_TYPE_OF, rhs);
	return TOY_OP_EOF;
}

static Toy_Opcode compound(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//read either an array or a dictionary into a literal node

	int iterations = 0; //count the number of entries iterated over

	//compound nodes to store what is read
	Toy_ASTNode* array = NULL;
	Toy_ASTNode* dictionary = NULL;

	while (!match(parser, TOY_TOKEN_BRACKET_RIGHT)) {
		//if empty dictionary, there will be a colon between the brackets
		if (iterations == 0 && match(parser, TOY_TOKEN_COLON)) {
			consume(parser, TOY_TOKEN_BRACKET_RIGHT, "Expected ']' at the end of empty dictionary definition");
			//emit an empty dictionary and finish
			Toy_emitASTNodeCompound(&dictionary, TOY_LITERAL_DICTIONARY);
			break;
		}

		if (iterations > 0) {
			consume(parser, TOY_TOKEN_COMMA, "Expected ',' in array or dictionary");
		}

		iterations++;

		Toy_ASTNode* left = NULL;
		Toy_ASTNode* right = NULL;

		//store the left
		parsePrecedence(parser, &left, PREC_PRIMARY);

		if (!left) { //error
			return TOY_OP_EOF;
		}

		//detect a dictionary
		if (match(parser, TOY_TOKEN_COLON)) {
			parsePrecedence(parser, &right, PREC_PRIMARY);

			if (!right) { //error
				Toy_freeASTNode(left);
				return TOY_OP_EOF;
			}

			//check we ARE defining a dictionary
			if (array) {
				error(parser, parser->previous, "Incorrect detection between array and dictionary");
				Toy_freeASTNode(array);
				return TOY_OP_EOF;
			}

			//init the dictionary
			if (!dictionary) {
				Toy_emitASTNodeCompound(&dictionary, TOY_LITERAL_DICTIONARY);
			}

			//grow the node if needed
			if (dictionary->compound.capacity < dictionary->compound.count + 1) {
				int oldCapacity = dictionary->compound.capacity;

				dictionary->compound.capacity = TOY_GROW_CAPACITY(oldCapacity);
				dictionary->compound.nodes = TOY_GROW_ARRAY(Toy_ASTNode, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
			}

			//store the left and right in the node
			Toy_setASTNodePair(&dictionary->compound.nodes[dictionary->compound.count++], left, right);
		}
		//detect an array
		else {
			//check we ARE defining an array
			if (dictionary) {
				error(parser, parser->current, "Incorrect detection between array and dictionary");
				Toy_freeASTNode(dictionary);
				return TOY_OP_EOF;
			}

			//init the array
			if (!array) {
				Toy_emitASTNodeCompound(&array, TOY_LITERAL_ARRAY);
			}

			//grow the node if needed
			if (array->compound.capacity < array->compound.count + 1) {
				int oldCapacity = array->compound.capacity;

				array->compound.capacity = TOY_GROW_CAPACITY(oldCapacity);
				array->compound.nodes = TOY_GROW_ARRAY(Toy_ASTNode, array->compound.nodes, oldCapacity, array->compound.capacity);
			}

			//copy into the array, and manually free the temp node
			array->compound.nodes[array->compound.count++] = *left;
			TOY_FREE(Toy_ASTNode, left);
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
		Toy_emitASTNodeCompound(&array, TOY_LITERAL_ARRAY);
		(*nodeHandle) = array;
	}


	//ignored
	return TOY_OP_EOF;
}

static Toy_Opcode string(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//handle strings
	switch(parser->previous.type) {
		case TOY_TOKEN_LITERAL_STRING: {
			//unescape valid escaped characters
			int strLength = 0;
			char* buffer = TOY_ALLOCATE(char, parser->previous.length);

			for (int i = 0; i < parser->previous.length; i++) {
				if (parser->previous.lexeme[i] != '\\') { //copy normally
					buffer[strLength++] = parser->previous.lexeme[i];
					continue;
				}

				//unescape based on the character
				switch(parser->previous.lexeme[++i]) {
					case 'n':
						buffer[strLength++] = '\n';
						break;
					case 't':
						buffer[strLength++] = '\t';
						break;
					case '\\':
						buffer[strLength++] = '\\';
						break;
					case '"':
						buffer[strLength++] = '"';
						break;
					default: {
						char msg[256];
						snprintf(msg, 256, TOY_CC_ERROR "Unrecognized escape character %c in string" TOY_CC_RESET, parser->previous.lexeme[++i]);
						error(parser, parser->previous, msg);
					}
				}
			}

			//for length safety
			if (strLength > TOY_MAX_STRING_LENGTH) {
				strLength = TOY_MAX_STRING_LENGTH;
				char msg[256];
				snprintf(msg, 256, TOY_CC_ERROR "Strings can only be a maximum of %d characters long" TOY_CC_RESET, TOY_MAX_STRING_LENGTH);
				error(parser, parser->previous, msg);
			}

			Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, strLength));
			TOY_FREE_ARRAY(char, buffer, parser->previous.length);
			Toy_emitASTNodeLiteral(nodeHandle, literal);
			Toy_freeLiteral(literal);
			return TOY_OP_EOF;
		}

		//TODO: interpolated strings

		default:
			error(parser, parser->previous, "Unexpected token passed to string precedence rule");
			return TOY_OP_EOF;
	}
}

static Toy_Opcode grouping(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//handle groupings with ()
	switch(parser->previous.type) {
		case TOY_TOKEN_PAREN_LEFT: {
			parsePrecedence(parser, nodeHandle, PREC_TERNARY);
			consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' at end of grouping");

			//process the result without optimisations
			Toy_emitASTNodeGrouping(nodeHandle);
			return TOY_OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to grouping precedence rule");
			return TOY_OP_EOF;
	}
}

static Toy_Opcode binary(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser);

	//binary() is an infix rule - so only get the RHS of the operator
	switch(parser->previous.type) {
		//arithmetic
		case TOY_TOKEN_PLUS: {
			parsePrecedence(parser, nodeHandle, PREC_TERM);
			return TOY_OP_ADDITION;
		}

		case TOY_TOKEN_MINUS: {
			parsePrecedence(parser, nodeHandle, PREC_TERM);
			return TOY_OP_SUBTRACTION;
		}

		case TOY_TOKEN_MULTIPLY: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return TOY_OP_MULTIPLICATION;
		}

		case TOY_TOKEN_DIVIDE: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return TOY_OP_DIVISION;
		}

		case TOY_TOKEN_MODULO: {
			parsePrecedence(parser, nodeHandle, PREC_FACTOR);
			return TOY_OP_MODULO;
		}

		//assignment
		case TOY_TOKEN_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_ASSIGN;
		}

		case TOY_TOKEN_PLUS_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_ADDITION_ASSIGN;
		}

		case TOY_TOKEN_MINUS_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_SUBTRACTION_ASSIGN;
		}

		case TOY_TOKEN_MULTIPLY_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_MULTIPLICATION_ASSIGN;
		}

		case TOY_TOKEN_DIVIDE_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_DIVISION_ASSIGN;
		}

		case TOY_TOKEN_MODULO_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return TOY_OP_VAR_MODULO_ASSIGN;
		}

		//comparison
		case TOY_TOKEN_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_EQUAL;
		}

		case TOY_TOKEN_NOT_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_NOT_EQUAL;
		}

		case TOY_TOKEN_LESS: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_LESS;
		}

		case TOY_TOKEN_LESS_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_LESS_EQUAL;
		}

		case TOY_TOKEN_GREATER: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_GREATER;
		}

		case TOY_TOKEN_GREATER_EQUAL: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_COMPARE_GREATER_EQUAL;
		}

		case TOY_TOKEN_AND: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_AND;
		}

		case TOY_TOKEN_OR: {
			parsePrecedence(parser, nodeHandle, PREC_COMPARISON);
			return TOY_OP_OR;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to binary precedence rule");
			return TOY_OP_EOF;
	}
}

static Toy_Opcode unary(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmpNode = NULL;

	if (parser->previous.type == TOY_TOKEN_MINUS) {
		//temp handle to potentially negate values
		parsePrecedence(parser, &tmpNode, PREC_TERNARY); //can be a literal

		//optimisation: check for negative literals
		if (tmpNode != NULL && tmpNode->type == TOY_AST_NODE_LITERAL && (TOY_IS_INTEGER(tmpNode->atomic.literal) || TOY_IS_FLOAT(tmpNode->atomic.literal))) {
			//negate directly, if int or float
			Toy_Literal lit = tmpNode->atomic.literal;

			if (TOY_IS_INTEGER(lit)) {
				lit = TOY_TO_INTEGER_LITERAL(-TOY_AS_INTEGER(lit));
			}

			if (TOY_IS_FLOAT(lit)) {
				lit = TOY_TO_FLOAT_LITERAL(-TOY_AS_FLOAT(lit));
			}

			tmpNode->atomic.literal = lit;
			*nodeHandle = tmpNode;

			return TOY_OP_EOF;
		}

		//check for negated boolean errors
		if (tmpNode != NULL && tmpNode->type == TOY_AST_NODE_LITERAL && TOY_IS_BOOLEAN(tmpNode->atomic.literal)) {
			error(parser, parser->previous, "Negative booleans are not allowed");
			return TOY_OP_EOF;
		}

		//actually emit the negation node
		Toy_emitASTNodeUnary(nodeHandle, TOY_OP_NEGATE, tmpNode);
	}

	else if (parser->previous.type == TOY_TOKEN_NOT) {
		//temp handle to potentially negate values
		parsePrecedence(parser, &tmpNode, PREC_CALL); //can be a literal, grouping, fn call, etc.

		//optimisation: check for inverted booleans
		if (tmpNode != NULL && tmpNode->type == TOY_AST_NODE_LITERAL && TOY_IS_BOOLEAN(tmpNode->atomic.literal)) {
			//negate directly, if boolean
			Toy_Literal lit = tmpNode->atomic.literal;

			lit = TOY_TO_BOOLEAN_LITERAL(!TOY_AS_BOOLEAN(lit));

			tmpNode->atomic.literal = lit;
			*nodeHandle = tmpNode;

			return TOY_OP_EOF;
		}

		//actually emit the negation
		Toy_emitASTNodeUnary(nodeHandle, TOY_OP_INVERT, tmpNode);
	}

	else {
		error(parser, parser->previous, "Unexpected token passed to unary precedence rule");
		return TOY_OP_EOF;
	}

	return TOY_OP_EOF;
}

static char* removeChar(char* lexeme, int length, char c) {
	int resPos = 0;
	char* result = TOY_ALLOCATE(char, length + 1);

	for (int i = 0; i < length; i++) {
		if (lexeme[i] == c) {
			continue;
		}

		result[resPos++] = lexeme[i];
	}

	result[resPos] = '\0';
	return result;
}

static Toy_Opcode atomic(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	switch(parser->previous.type) {
		case TOY_TOKEN_NULL:
			Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_NULL_LITERAL);
			return TOY_OP_EOF;

		case TOY_TOKEN_LITERAL_TRUE:
			Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_BOOLEAN_LITERAL(true));
			return TOY_OP_EOF;

		case TOY_TOKEN_LITERAL_FALSE:
			Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_BOOLEAN_LITERAL(false));
			return TOY_OP_EOF;

		case TOY_TOKEN_LITERAL_INTEGER: {
			int value = 0;
			char* lexeme = removeChar(parser->previous.lexeme, parser->previous.length, '_');
			sscanf(lexeme, "%d", &value);
			TOY_FREE_ARRAY(char, lexeme, parser->previous.length + 1);
			Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_INTEGER_LITERAL(value));
			return TOY_OP_EOF;
		}

		case TOY_TOKEN_LITERAL_FLOAT: {
			float value = 0;
			char* lexeme = removeChar(parser->previous.lexeme, parser->previous.length, '_');
			sscanf(lexeme, "%f", &value);
			TOY_FREE_ARRAY(char, lexeme, parser->previous.length + 1);
			Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_FLOAT_LITERAL(value));
			return TOY_OP_EOF;
		}

		case TOY_TOKEN_TYPE: {
			if (match(parser, TOY_TOKEN_CONST)) {
				Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_TYPE_LITERAL(TOY_LITERAL_TYPE, true));
			}
			else {
				Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_TYPE_LITERAL(TOY_LITERAL_TYPE, false));
			}

			return TOY_OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to atomic precedence rule");
			return TOY_OP_EOF;
	}
}

static Toy_Opcode identifier(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//make a copy of the string
	Toy_Token identifierToken = parser->previous;

	if (identifierToken.type != TOY_TOKEN_IDENTIFIER) {
		error(parser, parser->previous, "Expected identifier");
		return TOY_OP_EOF;
	}

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(identifierToken.lexeme, length));
	Toy_emitASTNodeLiteral(nodeHandle, identifier);
	Toy_freeLiteral(identifier);

	return TOY_OP_EOF;
}

static Toy_Opcode castingPrefix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	switch(parser->previous.type) {
		case TOY_TOKEN_BOOLEAN: {
			Toy_Literal literal = TOY_TO_TYPE_LITERAL(TOY_LITERAL_BOOLEAN, false);
			Toy_emitASTNodeLiteral(nodeHandle, literal);
			Toy_freeLiteral(literal);
		}
		break;

		case TOY_TOKEN_INTEGER: {
			Toy_Literal literal = TOY_TO_TYPE_LITERAL(TOY_LITERAL_INTEGER, false);
			Toy_emitASTNodeLiteral(nodeHandle, literal);
			Toy_freeLiteral(literal);
		}
		break;

		case TOY_TOKEN_FLOAT: {
			Toy_Literal literal = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FLOAT, false);
			Toy_emitASTNodeLiteral(nodeHandle, literal);
			Toy_freeLiteral(literal);
		}
		break;

		case TOY_TOKEN_STRING: {
			Toy_Literal literal = TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, false);
			Toy_emitASTNodeLiteral(nodeHandle, literal);
			Toy_freeLiteral(literal);
		}
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to casting precedence rule");
			return TOY_OP_EOF;
	}

	return TOY_OP_EOF;
}

static Toy_Opcode castingInfix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser);

	//NOTE: using the precedence rules here
	switch(parser->previous.type) {
		case TOY_TOKEN_IDENTIFIER:
			identifier(parser, nodeHandle);
		break;

		case TOY_TOKEN_LITERAL_TRUE:
		case TOY_TOKEN_LITERAL_FALSE:
			atomic(parser, nodeHandle);
		break;

		case TOY_TOKEN_LITERAL_INTEGER:
			atomic(parser, nodeHandle);
		break;

		case TOY_TOKEN_LITERAL_FLOAT:
			atomic(parser, nodeHandle);
		break;

		case TOY_TOKEN_LITERAL_STRING:
			atomic(parser, nodeHandle);
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to casting infix precedence rule");
			return TOY_OP_EOF;
	}

	return TOY_OP_TYPE_CAST;
}

//TODO: fix these screwy names
static Toy_Opcode incrementPrefix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser);

	Toy_ASTNode* tmpNode = NULL;
	identifier(parser, &tmpNode);

	Toy_emitASTNodePrefixIncrement(nodeHandle, tmpNode->atomic.literal);

	Toy_freeASTNode(tmpNode);

	return TOY_OP_EOF;
}

static Toy_Opcode incrementInfix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmpNode = NULL;
	identifier(parser, &tmpNode);

	advance(parser);

	Toy_emitASTNodePostfixIncrement(nodeHandle, tmpNode->atomic.literal);

	Toy_freeASTNode(tmpNode);

	return TOY_OP_EOF;
}

static Toy_Opcode decrementPrefix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser);

	Toy_ASTNode* tmpNode = NULL;
	identifier(parser, &tmpNode); //weird

	Toy_emitASTNodePrefixDecrement(nodeHandle, tmpNode->atomic.literal);

	Toy_freeASTNode(tmpNode);

	return TOY_OP_EOF;
}

static Toy_Opcode decrementInfix(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmpNode = NULL;
	identifier(parser, &tmpNode);

	advance(parser);

	Toy_emitASTNodePostfixDecrement(nodeHandle, tmpNode->atomic.literal);

	Toy_freeASTNode(tmpNode);

	return TOY_OP_EOF;
}

static Toy_Opcode fnCall(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser); //skip the left paren

	//binary() is an infix rule - so only get the RHS of the operator
	switch(parser->previous.type) {
		//arithmetic
		case TOY_TOKEN_PAREN_LEFT: {
			Toy_ASTNode* arguments = NULL;
			Toy_emitASTNodeFnCollection(&arguments);

			//if there's arguments
			if (!match(parser, TOY_TOKEN_PAREN_RIGHT)) {
				//read each argument
				do {
					//emit the node to the argument list (grow the node if needed)
					if (arguments->fnCollection.capacity < arguments->fnCollection.count + 1) {
						int oldCapacity = arguments->fnCollection.capacity;

						arguments->fnCollection.capacity = TOY_GROW_CAPACITY(oldCapacity);
						arguments->fnCollection.nodes = TOY_GROW_ARRAY(Toy_ASTNode, arguments->fnCollection.nodes, oldCapacity, arguments->fnCollection.capacity);
					}

					Toy_ASTNode* tmpNode = NULL;
					parsePrecedence(parser, &tmpNode, PREC_TERNARY);

					//BUGFIX
					if (!tmpNode) {
						error(parser, parser->previous, "[internal] No token found in fnCall");
						return TOY_OP_EOF;
					}

					arguments->fnCollection.nodes[arguments->fnCollection.count++] = *tmpNode;
					TOY_FREE(Toy_ASTNode, tmpNode); //simply free the tmpNode, so you don't free the children
				} while(match(parser, TOY_TOKEN_COMMA));

				consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' at end of argument list");
			}

			//emit the call
			Toy_emitASTNodeFnCall(nodeHandle, arguments);

			return TOY_OP_FN_CALL;
		}
		break;

		default:
			error(parser, parser->previous, "Unexpected token passed to function call precedence rule");
			return TOY_OP_EOF;
	}

	return TOY_OP_EOF;
}

static Toy_Opcode indexAccess(Toy_Parser* parser, Toy_ASTNode** nodeHandle) { //TODO: fix indexing signalling
	advance(parser);

	//val[first : second : third]

	Toy_ASTNode* first = NULL;
	Toy_ASTNode* second = NULL;
	Toy_ASTNode* third = NULL;

	//booleans indicate blank slice indexing
	Toy_emitASTNodeLiteral(&first, TOY_TO_INDEX_BLANK_LITERAL);
	Toy_emitASTNodeLiteral(&second, TOY_TO_INDEX_BLANK_LITERAL);
	Toy_emitASTNodeLiteral(&third, TOY_TO_INDEX_BLANK_LITERAL);

	bool readFirst = false; //pattern matching is bullcrap

	//eat the first
	if (!match(parser, TOY_TOKEN_COLON)) {
		Toy_freeASTNode(first);
		parsePrecedence(parser, &first, PREC_TERNARY);
		match(parser, TOY_TOKEN_COLON);
		readFirst = true;
	}

	if (match(parser, TOY_TOKEN_BRACKET_RIGHT)) {

		if (readFirst) {
			Toy_freeASTNode(second);
			second = NULL;
		}

		Toy_freeASTNode(third);
		third = NULL;

		Toy_emitASTNodeIndex(nodeHandle, first, second, third);
		return TOY_OP_INDEX;
	}

	//eat the second
	if (!match(parser, TOY_TOKEN_COLON)) {
		Toy_freeASTNode(second);
		parsePrecedence(parser, &second, PREC_TERNARY);
		match(parser, TOY_TOKEN_COLON);
	}

	if (match(parser, TOY_TOKEN_BRACKET_RIGHT)) {
		Toy_freeASTNode(third);
		third = NULL;
		Toy_emitASTNodeIndex(nodeHandle, first, second, third);
		return TOY_OP_INDEX;
	}

	//eat the third
	Toy_freeASTNode(third);
	parsePrecedence(parser, &third, PREC_TERNARY);
	Toy_emitASTNodeIndex(nodeHandle, first, second, third);

	consume(parser, TOY_TOKEN_BRACKET_RIGHT, "Expected ']' in index notation");

	return TOY_OP_INDEX;
}

static Toy_Opcode question(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser); //for the question mark

	Toy_ASTNode* thenPath = NULL;
	Toy_ASTNode* elsePath = NULL;

	parsePrecedence(parser, &thenPath, PREC_TERNARY);
	consume(parser, TOY_TOKEN_COLON, "Expected ':' in ternary expression");
	parsePrecedence(parser, &elsePath, PREC_TERNARY);

	Toy_emitASTNodeTernary(nodeHandle, NULL, thenPath, elsePath);

	return TOY_OP_TERNARY;
}

static Toy_Opcode dot(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	advance(parser); //for the dot

	Toy_ASTNode* tmpNode = NULL;
	parsePrecedence(parser, &tmpNode, PREC_CALL);

	if (tmpNode == NULL || tmpNode->binary.right == NULL) {
		error(parser, parser->previous, "Expected function call after dot operator");
		return TOY_OP_EOF;
	}

	(*nodeHandle) = tmpNode;
	return TOY_OP_DOT; //signal that the function name and arguments are in the wrong order
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
	{NULL, NULL, PREC_NONE},// TOKEN_OPAQUE,
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
	{atomic, NULL, PREC_PRIMARY},// TOKEN_TYPE,
	{asType, NULL, PREC_CALL},// TOKEN_ASTYPE,
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
	{NULL, question, PREC_TERNARY}, //TOKEN_QUESTION,
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

ParseRule* getRule(Toy_TokenType type) {
	return &parseRules[type];
}

//optimisation: constant folding
static bool calcStaticBinaryArithmetic(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	switch((*nodeHandle)->binary.opcode) {
		case TOY_OP_ADDITION:
		case TOY_OP_SUBTRACTION:
		case TOY_OP_MULTIPLICATION:
		case TOY_OP_DIVISION:
		case TOY_OP_MODULO:
		case TOY_OP_COMPARE_EQUAL:
		case TOY_OP_COMPARE_NOT_EQUAL:
		case TOY_OP_COMPARE_LESS:
		case TOY_OP_COMPARE_LESS_EQUAL:
		case TOY_OP_COMPARE_GREATER:
		case TOY_OP_COMPARE_GREATER_EQUAL:
			break;

		default:
			return true;
	}

	//recurse to the left and right
	if ((*nodeHandle)->binary.left->type == TOY_AST_NODE_BINARY) {
		calcStaticBinaryArithmetic(parser, &(*nodeHandle)->binary.left);
	}

	if ((*nodeHandle)->binary.right->type == TOY_AST_NODE_BINARY) {
		calcStaticBinaryArithmetic(parser, &(*nodeHandle)->binary.right);
	}

	//make sure left and right are both literals
	if (!((*nodeHandle)->binary.left->type == TOY_AST_NODE_LITERAL && (*nodeHandle)->binary.right->type == TOY_AST_NODE_LITERAL)) {
		return true;
	}

	//evaluate
	Toy_Literal lhs = (*nodeHandle)->binary.left->atomic.literal;
	Toy_Literal rhs = (*nodeHandle)->binary.right->atomic.literal;
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	//special case for string concatenation ONLY
	if (TOY_IS_STRING(lhs) && TOY_IS_STRING(rhs) && (*nodeHandle)->binary.opcode == TOY_OP_ADDITION) {
		//check for overflow
		int totalLength = TOY_AS_STRING(lhs)->length + TOY_AS_STRING(rhs)->length;
		if (totalLength > TOY_MAX_STRING_LENGTH) {
			error(parser, parser->previous, "Can't concatenate these strings, result is too long (error found in constant folding)\n");
			return false;
		}

		//concat the strings
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, TOY_MAX_STRING_LENGTH, "%s%s", Toy_toCString(TOY_AS_STRING(lhs)), Toy_toCString(TOY_AS_STRING(rhs)));
		result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, totalLength));
	}

	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//maths based on types
	if(TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		switch((*nodeHandle)->binary.opcode) {
			case TOY_OP_ADDITION:
				result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) + TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_SUBTRACTION:
				result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) - TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_MULTIPLICATION:
				result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) * TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_DIVISION:
				if (TOY_AS_INTEGER(rhs) == 0) {
					error(parser, parser->previous, "Can't divide by zero (error found in constant folding)");
					return false;
				}
				result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) / TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_MODULO:
				if (TOY_AS_INTEGER(rhs) == 0) {
					error(parser, parser->previous, "Can't modulo by zero (error found in constant folding)");
					return false;
				}
				result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) % TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) == TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_NOT_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) != TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_LESS:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) < TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_LESS_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) <= TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_GREATER:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) > TOY_AS_INTEGER(rhs) );
			break;

			case TOY_OP_COMPARE_GREATER_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_INTEGER(lhs) >= TOY_AS_INTEGER(rhs) );
			break;

			default:
				error(parser, parser->previous, "[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
				return false;
		}
	}

	//catch bad modulo
	if ((TOY_IS_FLOAT(lhs) || TOY_IS_FLOAT(rhs)) && (*nodeHandle)->binary.opcode == TOY_OP_MODULO) {
		error(parser, parser->previous, "Bad arithmetic argument (modulo on floats not allowed)");
		return false;
	}

	if(TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		switch((*nodeHandle)->binary.opcode) {
			case TOY_OP_ADDITION:
				result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) + TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_SUBTRACTION:
				result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) - TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_MULTIPLICATION:
				result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) * TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_DIVISION:
				if (TOY_AS_FLOAT(rhs) == 0) {
					error(parser, parser->previous, "Can't divide by zero (error found in constant folding)");
					return false;
				}
				result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) / TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) == TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_NOT_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) != TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_LESS:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) < TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_LESS_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) <= TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_GREATER:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) > TOY_AS_FLOAT(rhs) );
			break;

			case TOY_OP_COMPARE_GREATER_EQUAL:
				result = TOY_TO_BOOLEAN_LITERAL( TOY_AS_FLOAT(lhs) >= TOY_AS_FLOAT(rhs) );
			break;

			default:
				error(parser, parser->previous, "[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
				return false;
		}
	}

	//nothing can be done to optimize
	if (TOY_IS_NULL(result)) {
		return true;
	}

	//optimize by converting this node into a literal node
	Toy_freeASTNode((*nodeHandle)->binary.left);
	Toy_freeASTNode((*nodeHandle)->binary.right);

	(*nodeHandle)->type = TOY_AST_NODE_LITERAL;
	(*nodeHandle)->atomic.literal = result;

	return true;
}

static void dottify(Toy_Parser* parser, Toy_ASTNode** nodeHandle) { //TODO: remove dot from the compiler entirely
	//only if this is chained from a higher binary "fn call"
	if ((*nodeHandle)->type == TOY_AST_NODE_BINARY) {
		if ((*nodeHandle)->binary.opcode == TOY_OP_FN_CALL) {
			(*nodeHandle)->binary.opcode = TOY_OP_DOT;
			(*nodeHandle)->binary.right->fnCall.argumentCount++;
		}
		dottify(parser, &(*nodeHandle)->binary.left);
		dottify(parser, &(*nodeHandle)->binary.right);
	}
}

static void parsePrecedence(Toy_Parser* parser, Toy_ASTNode** nodeHandle, PrecedenceRule rule) {
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

		Toy_ASTNode* rhsNode = NULL;
		const Toy_Opcode opcode = infixRule(parser, &rhsNode); //NOTE: infix rule must advance the parser

		if (opcode == TOY_OP_EOF) {
			Toy_freeASTNode(*nodeHandle);
			*nodeHandle = rhsNode;
			return; //we're done here
		}

		//BUGFIX: dot-chaining
		if (opcode == TOY_OP_DOT) {
			dottify(parser, &rhsNode);
		}

		//BUGFIX: ternary shorthand
		if (opcode == TOY_OP_TERNARY) {
			rhsNode->ternary.condition = *nodeHandle;
			*nodeHandle = rhsNode;
			continue;
		}

		Toy_emitASTNodeBinary(nodeHandle, rhsNode, opcode);

		//optimise away the constants
		if (!parser->panic && !calcStaticBinaryArithmetic(parser, nodeHandle)) {
			return;
		}
	}

	//if your precedence is below "assignment"
	if (canBeAssigned && match(parser, TOY_TOKEN_ASSIGN)) {
		error(parser, parser->current, "Invalid assignment target");
	}
}

//expressions
static void expression(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//delegate to the pratt table for expression precedence
	parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
}

//statements
static void blockStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//init
	Toy_emitASTNodeBlock(nodeHandle);

	//sub-scope, compile it and push it up in a node
	while (!match(parser, TOY_TOKEN_BRACE_RIGHT)) {
		if ((*nodeHandle)->block.capacity < (*nodeHandle)->block.count + 1) {
			int oldCapacity = (*nodeHandle)->block.capacity;

			(*nodeHandle)->block.capacity = TOY_GROW_CAPACITY(oldCapacity);
			(*nodeHandle)->block.nodes = TOY_GROW_ARRAY(Toy_ASTNode, (*nodeHandle)->block.nodes, oldCapacity, (*nodeHandle)->block.capacity);
		}

		Toy_ASTNode* tmpNode = NULL;

		//process the grammar rule for this line
		declaration(parser, &tmpNode);

		// Ground floor: perfumery / Stationery and leather goods / Wigs and haberdashery / Kitchenware and food / Going up!
		if (parser->panic) {
			return;
		}

		//BUGFIX: statements no longer require the existing node
		((*nodeHandle)->block.nodes[(*nodeHandle)->block.count++]) = *tmpNode;
		TOY_FREE(Toy_ASTNode, tmpNode); //simply free the tmpNode, so you don't free the children
	}
}

static void printStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//set the node info
	Toy_ASTNode* node = NULL;
	expression(parser, &node);
	Toy_emitASTNodeUnary(nodeHandle, TOY_OP_PRINT, node);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of print statement");
}

static void assertStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//set the node info
	(*nodeHandle) = TOY_ALLOCATE(Toy_ASTNode, 1); //special case, because I'm lazy
	(*nodeHandle)->type = TOY_AST_NODE_BINARY;
	(*nodeHandle)->binary.opcode = TOY_OP_ASSERT;

	parsePrecedence(parser, &((*nodeHandle)->binary.left), PREC_TERNARY);
	consume(parser, TOY_TOKEN_COMMA, "Expected ',' in assert statement");
	parsePrecedence(parser, &((*nodeHandle)->binary.right), PREC_TERNARY);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of assert statement");
}

static void ifStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* condition = NULL;
	Toy_ASTNode* thenPath = NULL;
	Toy_ASTNode* elsePath = NULL;

	//read the condition
	consume(parser, TOY_TOKEN_PAREN_LEFT, "Expected '(' at beginning of if clause");
	parsePrecedence(parser, &condition, PREC_TERNARY);

	//read the then path
	consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' at end of if clause");
	declaration(parser, &thenPath);

	//read the optional else path
	if (match(parser, TOY_TOKEN_ELSE)) {
		declaration(parser, &elsePath);
	}

	Toy_emitASTNodeIf(nodeHandle, condition, thenPath, elsePath);
}

static void whileStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* condition = NULL;
	Toy_ASTNode* thenPath = NULL;

	//read the condition
	consume(parser, TOY_TOKEN_PAREN_LEFT, "Expected '(' at beginning of while clause");
	parsePrecedence(parser, &condition, PREC_TERNARY);

	//read the then path
	consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' at end of while clause");
	declaration(parser, &thenPath);

	Toy_emitASTNodeWhile(nodeHandle, condition, thenPath);
}

static void forStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* preClause = NULL;
	Toy_ASTNode* condition = NULL;
	Toy_ASTNode* postClause = NULL;
	Toy_ASTNode* thenPath = NULL;

	//read the clauses
	consume(parser, TOY_TOKEN_PAREN_LEFT, "Expected '(' at beginning of for clause");

	declaration(parser, &preClause); //allow defining variables in the pre-clause

	parsePrecedence(parser, &condition, PREC_TERNARY);
	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' after condition of for clause");

	parsePrecedence(parser, &postClause, PREC_ASSIGNMENT);
	consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' at end of for clause");

	//read the path
	declaration(parser, &thenPath);

	Toy_emitASTNodeFor(nodeHandle, preClause, condition, postClause, thenPath);
}

static void breakStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_emitASTNodeBreak(nodeHandle);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of break statement");
}

static void continueStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_emitASTNodeContinue(nodeHandle);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of continue statement");
}

static void returnStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* returnValues = NULL;
	Toy_emitASTNodeFnCollection(&returnValues);

	if (!match(parser, TOY_TOKEN_SEMICOLON)) {
		do { //loop for multiple returns (disabled later in the pipeline)
			//append the node to the return list (grow the node if needed)
			if (returnValues->fnCollection.capacity < returnValues->fnCollection.count + 1) {
				int oldCapacity = returnValues->fnCollection.capacity;

				returnValues->fnCollection.capacity = TOY_GROW_CAPACITY(oldCapacity);
				returnValues->fnCollection.nodes = TOY_GROW_ARRAY(Toy_ASTNode, returnValues->fnCollection.nodes, oldCapacity, returnValues->fnCollection.capacity);
			}

			Toy_ASTNode* node = NULL;
			parsePrecedence(parser, &node, PREC_TERNARY);

			//BUGFIX
			if (!node) {
				error(parser, parser->previous, "[internal] No token found in return");
				return;
			}

			returnValues->fnCollection.nodes[returnValues->fnCollection.count++] = *node;
			TOY_FREE(Toy_ASTNode, node); //free manually
		} while(match(parser, TOY_TOKEN_COMMA));

		consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of return statement");
	}

	Toy_emitASTNodeFnReturn(nodeHandle, returnValues);
}

static void importStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//read the identifier
	Toy_ASTNode* node = NULL;
	advance(parser);
	identifier(parser, &node);

	if (node == NULL) {
		return;
	}

	Toy_Literal idn = Toy_copyLiteral(node->atomic.literal);
	Toy_freeASTNode(node);

	Toy_Literal alias = TOY_TO_NULL_LITERAL;

	if (match(parser, TOY_TOKEN_AS)) {
		Toy_ASTNode* node = NULL;
		advance(parser);
		identifier(parser, &node);
		alias = Toy_copyLiteral(node->atomic.literal);
		Toy_freeASTNode(node);
	}

	Toy_emitASTNodeImport(nodeHandle, idn, alias);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of import statement");

	Toy_freeLiteral(idn);
	Toy_freeLiteral(alias);
}

//precedence functions
static void expressionStmt(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//BUGFIX: check for empty statements
	if (match(parser, TOY_TOKEN_SEMICOLON)) {
		Toy_emitASTNodeLiteral(nodeHandle, TOY_TO_NULL_LITERAL);
		return;
	}

	Toy_ASTNode* ptr = NULL;
	expression(parser, &ptr);

	if (ptr != NULL) {
		*nodeHandle = ptr;
	}

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at the end of expression statement");
}

static void statement(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//block
	if (match(parser, TOY_TOKEN_BRACE_LEFT)) {
		blockStmt(parser, nodeHandle);
		return;
	}

	//print
	if (match(parser, TOY_TOKEN_PRINT)) {
		printStmt(parser, nodeHandle);
		return;
	}

	//assert
	if (match(parser, TOY_TOKEN_ASSERT)) {
		assertStmt(parser, nodeHandle);
		return;
	}

	//if-then-else
	if (match(parser, TOY_TOKEN_IF)) {
		ifStmt(parser, nodeHandle);
		return;
	}

	//while-then
	if (match(parser, TOY_TOKEN_WHILE)) {
		whileStmt(parser, nodeHandle);
		return;
	}

	//for-pre-clause-post-then
	if (match(parser, TOY_TOKEN_FOR)) {
		forStmt(parser, nodeHandle);
		return;
	}

	//break
	if (match(parser, TOY_TOKEN_BREAK)) {
		breakStmt(parser, nodeHandle);
		return;
	}

	//continue
	if (match(parser, TOY_TOKEN_CONTINUE)) {
		continueStmt(parser, nodeHandle);
		return;
	}

	//return
	if (match(parser, TOY_TOKEN_RETURN)) {
		returnStmt(parser, nodeHandle);
		return;
	}

	//import
	if (match(parser, TOY_TOKEN_IMPORT)) {
		importStmt(parser, nodeHandle);
		return;
	}

	//default
	expressionStmt(parser, nodeHandle);
}

//declarations and definitions
static Toy_Literal readTypeToLiteral(Toy_Parser* parser) {
	advance(parser);

	Toy_Literal literal = TOY_TO_TYPE_LITERAL(TOY_LITERAL_NULL, false);

	switch(parser->previous.type) {
		case TOY_TOKEN_NULL:
			//NO-OP
		break;

		case TOY_TOKEN_BOOLEAN:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_BOOLEAN;
		break;

		case TOY_TOKEN_INTEGER:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_INTEGER;
		break;

		case TOY_TOKEN_FLOAT:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_FLOAT;
		break;

		case TOY_TOKEN_STRING:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_STRING;
		break;

		//array, dictionary - read the sub-types
		case TOY_TOKEN_BRACKET_LEFT: {
			Toy_Literal l = readTypeToLiteral(parser);

			if (match(parser, TOY_TOKEN_COLON)) {
				Toy_Literal r = readTypeToLiteral(parser);

				TOY_TYPE_PUSH_SUBTYPE(&literal, l);
				TOY_TYPE_PUSH_SUBTYPE(&literal, r);

				TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_DICTIONARY;
			}
			else {
				TOY_TYPE_PUSH_SUBTYPE(&literal, l);

				TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_ARRAY;
			}

			consume(parser, TOY_TOKEN_BRACKET_RIGHT, "Expected ']' at end of type definition");
		}
		break;

		case TOY_TOKEN_FUNCTION:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_FUNCTION;
		break;

		case TOY_TOKEN_OPAQUE:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_OPAQUE;
		break;

		case TOY_TOKEN_ANY:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_ANY;
		break;

		//wtf
		case TOY_TOKEN_IDENTIFIER: {
			//duplicated from identifier()
			Toy_Token identifierToken = parser->previous;
			int length = identifierToken.length;
			//for safety
			if (length > 256) {
				length = 256;
				error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
			}
			literal = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(identifierToken.lexeme, length));
		}
		break;

		//WTF
		case TOY_TOKEN_TYPE:
			TOY_AS_TYPE(literal).typeOf = TOY_LITERAL_TYPE;
		break;

		default:
			error(parser, parser->previous, "Bad type signature");
			return TOY_TO_NULL_LITERAL;
	}

	//const follows the type
	if (match(parser, TOY_TOKEN_CONST)) {
		TOY_AS_TYPE(literal).constant = true;
	}

	return literal;
}

static void varDecl(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//read the identifier
	consume(parser, TOY_TOKEN_IDENTIFIER, "Expected identifier after var keyword");
	Toy_Token identifierToken = parser->previous;

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(identifierToken.lexeme, length));

	//read the type, if present
	Toy_Literal typeLiteral;
	if (match(parser, TOY_TOKEN_COLON)) {
		typeLiteral = readTypeToLiteral(parser);
	}
	else {
		//default to non-const any
		typeLiteral = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, false);
	}

	//variable definition is an expression
	Toy_ASTNode* expressionNode = NULL;
	if (match(parser, TOY_TOKEN_ASSIGN)) {
		expression(parser, &expressionNode);
	}
	else {
		//values are null by default
		Toy_emitASTNodeLiteral(&expressionNode, TOY_TO_NULL_LITERAL);
	}

	//TODO: static type checking?

	//declare it
	Toy_emitASTNodeVarDecl(nodeHandle, identifier, typeLiteral, expressionNode);

	consume(parser, TOY_TOKEN_SEMICOLON, "Expected ';' at end of var declaration");
}

static void fnDecl(Toy_Parser* parser, Toy_ASTNode** nodeHandle) {
	//read the identifier
	consume(parser, TOY_TOKEN_IDENTIFIER, "Expected identifier after fn keyword");
	Toy_Token identifierToken = parser->previous;

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(identifierToken.lexeme, length));

	//read the parameters and arity
	consume(parser, TOY_TOKEN_PAREN_LEFT, "Expected '(' after function identifier");

	//for holding the array of arguments
	Toy_ASTNode* argumentNode = NULL;
	Toy_emitASTNodeFnCollection(&argumentNode);

	//read args
	if (!match(parser, TOY_TOKEN_PAREN_RIGHT)) {
		do {
			//check for rest parameter
			if (match(parser, TOY_TOKEN_REST)) {
				//read the argument identifier
				consume(parser, TOY_TOKEN_IDENTIFIER, "Expected identifier as function argument");
				Toy_Token argIdentifierToken = parser->previous;

				int length = argIdentifierToken.length;

				//for safety
				if (length > 256) {
					length = 256;
					error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
				}

				Toy_Literal argIdentifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(argIdentifierToken.lexeme, length));

				//set the type (array of any types)
				Toy_Literal argTypeLiteral = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FUNCTION_ARG_REST, false);

				//emit the node to the argument list (grow the node if needed)
				if (argumentNode->fnCollection.capacity < argumentNode->fnCollection.count + 1) {
					int oldCapacity = argumentNode->fnCollection.capacity;

					argumentNode->fnCollection.capacity = TOY_GROW_CAPACITY(oldCapacity);
					argumentNode->fnCollection.nodes = TOY_GROW_ARRAY(Toy_ASTNode, argumentNode->fnCollection.nodes, oldCapacity, argumentNode->fnCollection.capacity);
				}

				//store the arg in the array
				Toy_ASTNode* literalNode = NULL;
				Toy_emitASTNodeVarDecl(&literalNode, argIdentifier, argTypeLiteral, NULL);

				argumentNode->fnCollection.nodes[argumentNode->fnCollection.count++] = *literalNode;
				TOY_FREE(Toy_ASTNode, literalNode);

				break;
			}

			//read the argument identifier
			consume(parser, TOY_TOKEN_IDENTIFIER, "Expected identifier as function argument");
			Toy_Token argIdentifierToken = parser->previous;

			int length = argIdentifierToken.length;

			//for safety
			if (length > 256) {
				length = 256;
				error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
			}

			Toy_Literal argIdentifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(argIdentifierToken.lexeme, length));

			//read optional type of the identifier
			Toy_Literal argTypeLiteral;
			if (match(parser, TOY_TOKEN_COLON)) {
				argTypeLiteral = readTypeToLiteral(parser);
			}
			else {
				//default to non-const any
				argTypeLiteral = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, false);
			}

			//emit the node to the argument list (grow the node if needed)
			if (argumentNode->fnCollection.capacity < argumentNode->fnCollection.count + 1) {
				int oldCapacity = argumentNode->fnCollection.capacity;

				argumentNode->fnCollection.capacity = TOY_GROW_CAPACITY(oldCapacity);
				argumentNode->fnCollection.nodes = TOY_GROW_ARRAY(Toy_ASTNode, argumentNode->fnCollection.nodes, oldCapacity, argumentNode->fnCollection.capacity);
			}

			//store the arg in the array
			Toy_ASTNode* literalNode = NULL;
			Toy_emitASTNodeVarDecl(&literalNode, argIdentifier, argTypeLiteral, NULL);

			argumentNode->fnCollection.nodes[argumentNode->fnCollection.count++] = *literalNode;
			TOY_FREE(Toy_ASTNode, literalNode);

		} while (match(parser, TOY_TOKEN_COMMA)); //if comma is read, continue

		consume(parser, TOY_TOKEN_PAREN_RIGHT, "Expected ')' after function argument list");
	}

	//read the return types, if present
	Toy_ASTNode* returnNode = NULL;
	Toy_emitASTNodeFnCollection(&returnNode);

	if (match(parser, TOY_TOKEN_COLON)) {
		do {
			//append the node to the return list (grow the node if needed)
			if (returnNode->fnCollection.capacity < returnNode->fnCollection.count + 1) {
				int oldCapacity = returnNode->fnCollection.capacity;

				returnNode->fnCollection.capacity = TOY_GROW_CAPACITY(oldCapacity);
				returnNode->fnCollection.nodes = TOY_GROW_ARRAY(Toy_ASTNode, returnNode->fnCollection.nodes, oldCapacity, returnNode->fnCollection.capacity);
			}

			Toy_ASTNode* literalNode = NULL;
			Toy_emitASTNodeLiteral(&literalNode, readTypeToLiteral(parser));

			returnNode->fnCollection.nodes[returnNode->fnCollection.count++] = *literalNode;
			TOY_FREE(Toy_ASTNode, literalNode);
		} while(match(parser, TOY_TOKEN_COMMA));
	}

	//read the function body
	consume(parser, TOY_TOKEN_BRACE_LEFT, "Expected '{' after return list");

	Toy_ASTNode* blockNode = NULL;
	blockStmt(parser, &blockNode);

	//declare it
	Toy_emitASTNodeFnDecl(nodeHandle, identifier, argumentNode, returnNode, blockNode);
}

static void declaration(Toy_Parser* parser, Toy_ASTNode** nodeHandle) { //assume nodeHandle holds a blank node
	//variable declarations
	if (match(parser, TOY_TOKEN_VAR)) {
		varDecl(parser, nodeHandle);
	}
	else if (match(parser, TOY_TOKEN_FUNCTION)) {
		fnDecl(parser, nodeHandle);
	}
	else {
		statement(parser, nodeHandle);
	}
}

//exposed functions
void Toy_initParser(Toy_Parser* parser, Toy_Lexer* lexer) {
	parser->lexer = lexer;
	parser->error = false;
	parser->panic = false;

	parser->previous.type = TOY_TOKEN_NULL;
	parser->current.type = TOY_TOKEN_NULL;
	advance(parser);
}

void Toy_freeParser(Toy_Parser* parser) {
	parser->lexer = NULL;
	parser->error = false;
	parser->panic = false;

	parser->previous.type = TOY_TOKEN_NULL;
	parser->current.type = TOY_TOKEN_NULL;
}

Toy_ASTNode* Toy_scanParser(Toy_Parser* parser) {
	//check for EOF
	if (match(parser, TOY_TOKEN_EOF)) {
		return NULL;
	}

	//returns nodes on the heap
	Toy_ASTNode* node = NULL;

	//process the grammar rule for this line
	declaration(parser, &node);

	if (parser->panic) {
		synchronize(parser);
		//return an error node for this iteration
		Toy_freeASTNode(node);
		node = TOY_ALLOCATE(Toy_ASTNode, 1);
		node->type = TOY_AST_NODE_ERROR;
	}

	return node;
}

