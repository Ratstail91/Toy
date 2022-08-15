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
	if (command.verbose) {
		printf(ERROR "synchronizing\n" RESET);
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
			case TOKEN_TYPE:
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

typedef Opcode (*ParseFn)(Parser* parser, Node** nodeHandle, bool canBeAssigned);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	PrecedenceRule precedence;
} ParseRule;

ParseRule parseRules[];

//forward declarations
static void declaration(Parser* parser, Node** nodeHandle);
static void parsePrecedence(Parser* parser, Node** nodeHandle, PrecedenceRule rule);

//the expression rules
static Opcode compound(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
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
			Node* pair = NULL;
			emitNodePair(&pair, left, right);
			dictionary->compound.nodes[dictionary->compound.count++] = *pair;
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

			//store the left in the array
			array->compound.nodes[array->compound.count++] = *left;
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

static Opcode string(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//handle strings
	switch(parser->previous.type) {
		case TOKEN_LITERAL_STRING: {
			int length = parser->previous.length;

			//for safety
			if (length > 4096) {
				length = 4096;
				error(parser, parser->previous, "Strings can only be a maximum of 4096 characters long");
			}

			emitNodeLiteral(nodeHandle, TO_STRING_LITERAL(copyString(parser->previous.lexeme, length)));
			return OP_EOF;
		}

		//TODO: interpolated strings

		default:
			error(parser, parser->previous, "Unexpected token passed to string precedence rule");
			return OP_EOF;
	}
}

static Opcode grouping(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//handle groupings with ()
	switch(parser->previous.type) {
		case TOKEN_PAREN_LEFT: {
			Node* tmpNode = NULL;
			parsePrecedence(parser, &tmpNode, PREC_TERNARY);
			consume(parser, TOKEN_PAREN_RIGHT, "Expected ')' at end of grouping");

			//if it's just a literal, don't need a grouping
			if (command.optimize >= 1 && tmpNode->type == NODE_LITERAL) {
				(*nodeHandle) = tmpNode;
				return OP_EOF;
			}

			//process the result without optimisations
			emitNodeGrouping(nodeHandle);
			nodeHandle = &((*nodeHandle)->unary.child); //re-align after append
			(*nodeHandle) = tmpNode;
			return OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to grouping precedence rule");
			return OP_EOF;
	}
}

static Opcode binary(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	advance(parser);

	//binary() is an infix rule - so only get the RHS of the operator
	switch(parser->previous.type) {
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

		case TOKEN_ASSIGN: {
			parsePrecedence(parser, nodeHandle, PREC_ASSIGNMENT);
			return OP_VAR_ASSIGN;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to binary precedence rule");
			return OP_EOF;
	}
}

static Opcode unary(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	switch(parser->previous.type) {
		case TOKEN_MINUS: {
			//temp handle to potentially negate values
			Node* tmpNode = NULL;
			parsePrecedence(parser, &tmpNode, PREC_TERNARY); //can be a literal

			//check for negative literals (optimisation)
			if (command.optimize >= 1 && tmpNode->type == NODE_LITERAL) {
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

			//process the literal without optimizations
			if (tmpNode->type == NODE_LITERAL) {
				emitNodeUnary(nodeHandle, OP_NEGATE);
				nodeHandle = &((*nodeHandle)->unary.child); //re-align after append
				(*nodeHandle) = tmpNode; //set negate's child to the literal
				return OP_EOF;
			}

			error(parser, parser->previous, "Unexpected token passed to unary minus precedence rule");
			return OP_EOF;
		}

		default:
			error(parser, parser->previous, "Unexpected token passed to unary precedence rule");
			return OP_EOF;
	}
}

static Opcode atomic(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
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

		default:
			error(parser, parser->previous, "Unexpected token passed to atomic precedence rule");
			return OP_EOF;
	}
}

static Opcode identifier(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//make a copy of the string
	Token identifierToken = parser->previous;

	int length = identifierToken.length;

	//for safety
	if (length > 256) {
		length = 256;
		error(parser, parser->previous, "Identifiers can only be a maximum of 256 characters long");
	}

	char* cpy = copyString(identifierToken.lexeme, length);
	Literal identifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

	emitNodeLiteral(nodeHandle, identifier);

	return OP_EOF;
}

ParseRule parseRules[] = { //must match the token types
	//types
	{atomic, NULL, PREC_PRIMARY},// TOKEN_NULL,
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
	{identifier, NULL, PREC_PRIMARY},// TOKEN_IDENTIFIER,
	{atomic, NULL, PREC_PRIMARY},// TOKEN_LITERAL_TRUE,
	{atomic, NULL, PREC_PRIMARY},// TOKEN_LITERAL_FALSE,
	{atomic, NULL, PREC_PRIMARY},// TOKEN_LITERAL_INTEGER,
	{atomic, NULL, PREC_PRIMARY},// TOKEN_LITERAL_FLOAT,
	{string, NULL, PREC_PRIMARY},// TOKEN_LITERAL_STRING,

	//math operators
	{NULL, binary, PREC_TERM},// TOKEN_PLUS,
	{unary, binary, PREC_TERM},// TOKEN_MINUS,
	{NULL, binary, PREC_TERM},// TOKEN_MULTIPLY,
	{NULL, binary, PREC_TERM},// TOKEN_DIVIDE,
	{NULL, binary, PREC_TERM},// TOKEN_MODULO,
	{NULL, NULL, PREC_NONE},// TOKEN_PLUS_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MINUS_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MULTIPLY_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_DIVIDE_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_MODULO_ASSIGN,
	{NULL, NULL, PREC_NONE},// TOKEN_PLUS_PLUS,
	{NULL, NULL, PREC_NONE},// TOKEN_MINUS_MINUS,
	{NULL, binary, PREC_ASSIGNMENT},// TOKEN_ASSIGN,

	//logical operators
	{grouping, NULL, PREC_CALL},// TOKEN_PAREN_LEFT,
	{NULL, NULL, PREC_NONE},// TOKEN_PAREN_RIGHT,
	{compound, NULL, PREC_CALL},// TOKEN_BRACKET_LEFT,
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
	{NULL, NULL, PREC_NONE},// TOKEN_COLON,
	{NULL, NULL, PREC_NONE},// TOKEN_SEMICOLON,
	{NULL, NULL, PREC_CALL},// TOKEN_COMMA,
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

//constant folding
static bool calcStaticBinaryArithmetic(Parser* parser, Node** nodeHandle) {
	switch((*nodeHandle)->binary.opcode) {
		case OP_ADDITION:
		case OP_SUBTRACTION:
		case OP_MULTIPLICATION:
		case OP_DIVISION:
		case OP_MODULO:
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

			default:
				printf("[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
				return false;
		}
	}

	//catch bad modulo
	if ((IS_FLOAT(lhs) || IS_FLOAT(rhs)) && (*nodeHandle)->binary.opcode == OP_MODULO) {
		printf("Bad arithmetic argument (modulo on floats not allowed)");
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

			default:
				printf("[internal] bad opcode argument passed to calcStaticBinaryArithmetic()");
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
	prefixRule(parser, nodeHandle, canBeAssigned); //ignore the returned opcode

	//infix rules are left-recursive
	while (rule <= getRule(parser->current.type)->precedence) {
		ParseFn infixRule = getRule(parser->current.type)->infix;

		if (infixRule == NULL) {
			*nodeHandle = NULL; //the handle's value MUST be set to null for error handling
			error(parser, parser->current, "Expected operator");
			return;
		}

		Node* rhsNode = NULL;
		const Opcode opcode = infixRule(parser, &rhsNode, canBeAssigned); //NOTE: infix rule must advance the parser
		emitNodeBinary(nodeHandle, rhsNode, opcode);

		if (command.optimize >= 1 && !calcStaticBinaryArithmetic(parser, nodeHandle)) {
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
	(*nodeHandle)->type = NODE_BLOCK;
	(*nodeHandle)->block.nodes = NULL;
	(*nodeHandle)->block.capacity = 0;
	(*nodeHandle)->block.count = 0;

	//sub-scope, compile it and push it up in a node
	while (!match(parser, TOKEN_BRACE_RIGHT)) {
		if ((*nodeHandle)->block.capacity < (*nodeHandle)->block.count + 1) {
			int oldCapacity = (*nodeHandle)->block.capacity;

			(*nodeHandle)->block.capacity = GROW_CAPACITY(oldCapacity);
			(*nodeHandle)->block.nodes = GROW_ARRAY(Node, (*nodeHandle)->block.nodes, oldCapacity, (*nodeHandle)->block.capacity);
		}

		//use the next node in sequence
		(*nodeHandle)->block.nodes[(*nodeHandle)->block.count].type = NODE_ERROR; //BUGFIX: so freeing won't break the damn thing

		Node* ptr = &((*nodeHandle)->block.nodes[(*nodeHandle)->block.count]);

		//process the grammar rule for this line
		declaration(parser, &ptr);

		//BUGFIX: if ptr has been re-assigned, copy the new value into the block's child
		if (&((*nodeHandle)->block.nodes[(*nodeHandle)->block.count]) != ptr) {
			((*nodeHandle)->block.nodes[(*nodeHandle)->block.count]) = *ptr;
			FREE(Node, ptr);
		}

		(*nodeHandle)->block.count++;

		// Ground floor: perfumery / Stationery and leather goods / Wigs and haberdashery / Kitchenware and food / Going up!
		if (parser->panic) {
			return;
		}
	}
}

static void printStmt(Parser* parser, Node** nodeHandle) {
	//set the node info
	(*nodeHandle)->type = NODE_UNARY;
	(*nodeHandle)->unary.opcode = OP_PRINT;
	expression(parser, &((*nodeHandle)->unary.child));

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of print statement");
}

static void assertStmt(Parser* parser, Node** nodeHandle) {
	//set the node info
	(*nodeHandle)->type = NODE_BINARY;
	(*nodeHandle)->unary.opcode = OP_ASSERT;

	parsePrecedence(parser, &((*nodeHandle)->binary.left), PREC_PRIMARY);
	consume(parser, TOKEN_COMMA, "Expected ',' in assert statement");
	parsePrecedence(parser, &((*nodeHandle)->binary.right), PREC_PRIMARY);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of assert statement");
}

//precedence functions
static void expressionStmt(Parser* parser, Node** nodeHandle) {
	//BUGFIX: check for empty statements
	if (match(parser, TOKEN_SEMICOLON)) {
		(*nodeHandle)->type = NODE_LITERAL;
		(*nodeHandle)->atomic.literal = TO_NULL_LITERAL;
		return;
	}

	//BUGFIX: statements assume the node exists, expressions assume it doens't
	Node* ptr = NULL;
	expression(parser, &ptr);

	if (ptr != NULL) {
		**nodeHandle = *ptr;
		FREE(Node, ptr); //BUGFIX: this thread of execution is nuts
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

	//default
	expressionStmt(parser, nodeHandle);
}

//declarations and definitions
static Literal readTypeToLiteral(Parser* parser) {
	advance(parser);

	Literal literal = TO_TYPE_LITERAL(MASK_ANY);

	switch(parser->previous.type) {
		case TOKEN_BOOLEAN:
			AS_TYPE(literal).mask |= MASK_BOOLEAN;
		break;

		case TOKEN_INTEGER:
			AS_TYPE(literal).mask |= MASK_INTEGER;
		break;

		case TOKEN_FLOAT:
			AS_TYPE(literal).mask |= MASK_FLOAT;
		break;

		case TOKEN_STRING:
			AS_TYPE(literal).mask |= MASK_STRING;
		break;

		//array, dictionary - read the sub-types
		case TOKEN_BRACKET_LEFT: {
			Literal l = readTypeToLiteral(parser);

			if (match(parser, TOKEN_COMMA)) {
				Literal r = readTypeToLiteral(parser);

				AS_TYPE(literal).subtypes = ALLOCATE(Literal, 2);
				AS_TYPE(literal).capacity = 2;
				AS_TYPE(literal).count = 2;

				((Literal*)(AS_TYPE(literal).subtypes))[0] = l;
				((Literal*)(AS_TYPE(literal).subtypes))[1] = r;

				AS_TYPE(literal).mask |= MASK_DICTIONARY;
			}
			else {
				AS_TYPE(literal).subtypes = ALLOCATE(Literal, 1);
				AS_TYPE(literal).capacity = 1;
				AS_TYPE(literal).count = 1;

				//append the "l" literal
				((Literal*)(AS_TYPE(literal).subtypes))[0] = l;

				AS_TYPE(literal).mask |= MASK_ARRAY;
			}

			consume(parser, TOKEN_BRACKET_RIGHT, "Expected ']' at end of type definition");
		}
		break;

		//TODO: function

		default:
			error(parser, parser->previous, "Bad type signature");
			return TO_NULL_LITERAL;
	}

	//const follows the type
	if (match(parser, TOKEN_CONST)) {
		AS_TYPE(literal).mask |= MASK_CONST;
	}

	return literal;
}

static void varDecl(Parser* parser, Node** nodeHandle) {
	//read the identifier
	consume(parser, TOKEN_IDENTIFIER, "Expected identifier after var keyword");
	Token identifierToken = parser->previous;

	char* cpy = copyString(identifierToken.lexeme, identifierToken.length);
	Literal identifier = _toIdentifierLiteral(cpy, strlen(cpy)); //BUGFIX: use this instead of the macro

	//read the type, if present
	Literal typeLiteral;
	if (match(parser, TOKEN_COLON)) {
		typeLiteral = readTypeToLiteral(parser);
	}
	else {
		typeLiteral = TO_TYPE_LITERAL(MASK_ANY);
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

static void declaration(Parser* parser, Node** nodeHandle) { //assume nodeHandle holds a blank node
	//variable declarations
	if (match(parser, TOKEN_VAR)) {
		varDecl(parser, nodeHandle);
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
	Node* node = ALLOCATE(Node, 1);
	node->type = NODE_ERROR; //BUGFIX: so freeing won't break the damn thing

	//process the grammar rule for this line
	declaration(parser, &node);

	if (parser->panic) {
		synchronize(parser);
		//return an error node for this iteration
		node = ALLOCATE(Node, 1);
		node->type = NODE_ERROR;
	}

	return node;
}
