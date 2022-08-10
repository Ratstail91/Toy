#pragma once

#include "literal.h"
#include "opcodes.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node Node;

typedef enum NodeType {
	NODE_ERROR,
	NODE_LITERAL, //a simple value
	NODE_UNARY, //one child
	NODE_BINARY, //two children, left and right
	NODE_GROUPING, //one child
	NODE_BLOCK, //contains sub-node array
	NODE_VAR_TYPES, //contains a type mask and a sub-node array for compound types
	NODE_VAR_DECL, //contains identifier literal, typenode, expression definition
	// NODE_CONDITIONAL, //three children: conditional, then path, else path
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

typedef struct NodeGrouping {
	NodeType type;
	Node* child;
} NodeGrouping;

typedef struct NodeBlock {
	NodeType type;
	Node* nodes;
	int capacity;
	int count;
} NodeBlock;

typedef struct NodeVarTypes {
	NodeType type;
	unsigned char mask;
	Node* nodes;
	int capacity;
	int count;
} NodeVarTypes;

typedef struct NodeVarDecl {
	NodeType type;
	Literal identifier;
	Node* varType;
	Node* expression;
} NodeVarDecl;

union _node {
	NodeType type;
	NodeLiteral atomic;
	NodeUnary unary;
	NodeBinary binary;
	NodeGrouping grouping;
	NodeBlock block;
	NodeVarTypes varTypes;
	NodeVarDecl varDecl;
};

void freeNode(Node* node);
void emitNodeLiteral(Node** nodeHandle, Literal literal);
void emitNodeUnary(Node** nodeHandle, Opcode opcode);
void emitNodeBinary(Node** nodeHandle, Node* rhs, Opcode opcode);
void emitNodeGrouping(Node** nodeHandle);
void emitNodeBlock(Node** nodeHandle);
void emitNodeVarTypes(Node** nodeHandle, unsigned char mask);
void emitNodeVarDecl(Node** nodeHandle, Literal identifier, Node* varType, Node* expression);

void printNode(Node* node);

