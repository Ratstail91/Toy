#pragma once

#include "opcodes.h"
#include "literal.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node Node;

typedef enum NodeType {
	NODE_ATOMIC, //a simple value
	NODE_UNARY, //one child
	NODE_BINARY, //two children, left and right
	// NODE_GROUPING,
} NodeType;

typedef struct NodeAtomic {
	NodeType type;
	Literal literal;
} NodeAtomic;

typedef struct NodeUnary {
	NodeType type;
	Node* child;
} NodeUnary;

typedef struct NodeBinary {
	NodeType type;
	Node* left;
	Node* right;
} NodeBinary;

union _node {
	NodeType type;
	NodeAtomic atomic;
	NodeUnary unary;
	NodeBinary binary;
};

void freeNode(Node* node);
void emitAtomicLiteral(Node** nodeHandle, Literal literal);

void printNode(Node* node);

