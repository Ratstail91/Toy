#include "parser.h"

#include "common.h"

#include "memory.h"
#include "literal.h"
#include "opcodes.h"

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
		printf(parser->lexer->source);
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

		//detect a dictionary
		if (match(parser, TOKEN_COLON)) {
			parsePrecedence(parser, &right, PREC_PRIMARY);

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
		case TOKEN_LITERAL_STRING:
			emitNodeLiteral(nodeHandle, TO_STRING_LITERAL(copyString(parser->previous.lexeme, parser->previous.length)));
			return OP_EOF;

		//TODO: interpolated strings

		default:
			error(parser, parser->previous, "Unexpected token passed to string precedence rule");
			return OP_EOF;
	}
}

static Opcode grouping(Parser* parser, Node** nodeHandle, bool canBeAssigned) {
	//handle three diffent types of groupings: (), {}, []
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
	{NULL, NULL, PREC_NONE},// TOKEN_IDENTIFIER,
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
	{NULL, NULL, PREC_NONE},// TOKEN_ASSIGN,
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

//static analasys
static bool calcStaticBinaryArithmetic(Node** nodeHandle) {
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
		calcStaticBinaryArithmetic(&(*nodeHandle)->binary.left);
	}

	if ((*nodeHandle)->binary.right->type == NODE_BINARY) {
		calcStaticBinaryArithmetic(&(*nodeHandle)->binary.right);
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
				result = TO_INTEGER_LITERAL( AS_INTEGER(lhs) / AS_INTEGER(rhs) );
			break;

			case OP_MODULO:
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

	//optimize by converting this node into a literal
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

		if (command.optimize >= 1 && !calcStaticBinaryArithmetic(nodeHandle)) {
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
static void blockStmt(Parser* parser, Node* node) {
	//init
	node->type = NODE_BLOCK;
	node->block.nodes = NULL;
	node->block.capacity = 0;
	node->block.count = 0;

	//sub-scope, compile it and push it up in a node
	while (!match(parser, TOKEN_BRACE_RIGHT)) {
		if (node->block.capacity < node->block.count + 1) {
			int oldCapacity = node->block.capacity;

			node->block.capacity = GROW_CAPACITY(oldCapacity);
			node->block.nodes = GROW_ARRAY(Node, node->block.nodes, oldCapacity, node->block.capacity);
		}

		//use the next node in sequence
		node->block.nodes[node->block.count].type = NODE_ERROR; //BUGFIX: so freeing won't break the damn thing

		Node* ptr = &(node->block.nodes[node->block.count++]);

		//process the grammar rule for this line
		declaration(parser, &ptr);

		// Ground floor: perfumery / Stationery and leather goods / Wigs and haberdashery / Kitchenware and food / Going up!
		if (parser->panic) {
			return;
		}
	}
}

static void printStmt(Parser* parser, Node* node) {
	//set the node info
	node->type = NODE_UNARY;
	node->unary.opcode = OP_PRINT;
	expression(parser, &(node->unary.child));

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of print statement");
}

static void assertStmt(Parser* parser, Node* node) {
	//set the node info
	node->type = NODE_BINARY;
	node->unary.opcode = OP_ASSERT;

	parsePrecedence(parser, &(node->binary.left), PREC_PRIMARY);
	consume(parser, TOKEN_COMMA, "Expected ',' in assert statement");
	parsePrecedence(parser, &(node->binary.right), PREC_PRIMARY);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of assert statement");
}

//precedence functions
static void expressionStmt(Parser* parser, Node* node) {
	error(parser, parser->previous, "Expression statements not yet implemented");
}

static void statement(Parser* parser, Node* node) {
	//block
	if (match(parser, TOKEN_BRACE_LEFT)) {
		blockStmt(parser, node);
		return;
	}

	//print
	if (match(parser, TOKEN_PRINT)) {
		printStmt(parser, node);
		return;
	}

	//assert
	if (match(parser, TOKEN_ASSERT)) {
		assertStmt(parser, node);
		return;
	}

	//default
	expressionStmt(parser, node);
}

//declarations and definitions
static void readVarType(Parser* parser, Node** nodeHandle) {
	//TODO: custom types with the "type" keyword
	advance(parser);

	unsigned char typeMask = 0;

	Node* left = NULL;
	Node* right = NULL;

	switch(parser->previous.type) {
		case TOKEN_BOOLEAN:
			typeMask |= MASK_BOOLEAN;
		break;

		case TOKEN_INTEGER:
			typeMask |= MASK_INTEGER;
		break;

		case TOKEN_FLOAT:
			typeMask |= MASK_FLOAT;
		break;

		case TOKEN_STRING:
			typeMask |= MASK_STRING;
		break;

		//array, dictionary - read the sub-types
		case TOKEN_BRACKET_LEFT:
			//at least 1 type required
			readVarType(parser, &left);

			if (match(parser, TOKEN_COMMA)) {
				//if there's 2 types, it's a dictionary
				readVarType(parser, &right);
				typeMask |= MASK_DICTIONARY;
			}
			else {
				//else it's just an array
				typeMask |= MASK_ARRAY;
			}
			consume(parser, TOKEN_BRACKET_RIGHT, "Expected ']' at end of type definition");
		break;

		case TOKEN_ANY:
			typeMask |= MASK_ANY;
		break;

		//TODO: function

		default:
			error(parser, parser->previous, "Bad type");
			return;
	}

	//const follows the type
	if (match(parser, TOKEN_CONST)) {
		typeMask |= MASK_CONST;
	}

	//generate the node
	emitNodeVarTypes(nodeHandle, typeMask);

	//check for sub-nodes
	if (left) {
		int oldCapacity = (*nodeHandle)->varTypes.capacity;

		(*nodeHandle)->varTypes.capacity = GROW_CAPACITY(oldCapacity);
		(*nodeHandle)->varTypes.nodes = GROW_ARRAY(Node, (*nodeHandle)->varTypes.nodes, oldCapacity, (*nodeHandle)->varTypes.capacity);

		//push left to the array
		*((*nodeHandle)->varTypes.nodes) = *left;

		//append the other one too
		if (right) {
			*((*nodeHandle)->varTypes.nodes + 1) = *right;
		}
	}
}

static void varDecl(Parser* parser, Node** nodeHandle) {
	//read the identifier
	consume(parser, TOKEN_IDENTIFIER, "Expected identifier after var keyword");
	Token identifierToken = parser->previous;

	//read the type, if present
	Node* typeNode = NULL;
	if (match(parser, TOKEN_COLON)) {
		readVarType(parser, &typeNode);
	}

	//variable definition is an expression
	Node* expressionNode = NULL;
	if (match(parser, TOKEN_ASSIGN)) {
		expression(parser, &expressionNode);
	}

	//TODO: compile-time static type check

	//finally
	emitNodeVarDecl(nodeHandle, TO_IDENTIFIER_LITERAL(identifierToken.lexeme), typeNode, expressionNode);

	consume(parser, TOKEN_SEMICOLON, "Expected ';' at end of var declaration");
}

static void declaration(Parser* parser, Node** nodeHandle) { //assume nodeHandle holds a blank node
	//variable declarations
	if (match(parser, TOKEN_VAR)) {
		varDecl(parser, nodeHandle);
	}
	else {
		statement(parser, *nodeHandle);
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
