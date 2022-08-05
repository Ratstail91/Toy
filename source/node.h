#pragma once

#include "literal.h"
#include "opcodes.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node Node;

typedef enum NodeType {
	NODE_LITERAL, //a simple value
	NODE_UNARY, //one child
	NODE_BINARY, //two children, left and right
	// NODE_GROUPING,
} NodeType;

typedef struct NodeLiteral {
	NodeType type;
	Literal literal;
} NodeLiteral;

typedef struct NodeUnary {
	NodeType type;
	Opcode opcode;
	Node* child;
} NodeUnary;

typedef struct NodeBinary {
	NodeType type;
	Opcode opcode;
	Node* left;
	Node* right;
} NodeBinary;

union _node {
	NodeType type;
	NodeLiteral atomic;
	NodeUnary unary;
	NodeBinary binary;
};

void freeNode(Node* node);
void emitNodeLiteral(Node** nodeHandle, Literal literal);
void emitNodeUnary(Node** nodeHandle, Opcode opcode);

void printNode(Node* node);

