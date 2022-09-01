#pragma once

#include "literal.h"
#include "opcodes.h"
#include "token_types.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node Node;

typedef enum NodeType {
	NODE_ERROR,
	NODE_LITERAL, //a simple value
	NODE_UNARY, //one child + opcode
	NODE_BINARY, //two children, left and right + opcode
	NODE_GROUPING, //one child
	NODE_BLOCK, //contains a sub-node array
	NODE_COMPOUND, //contains a sub-node array
	NODE_PAIR, //contains a left and right
	NODE_VAR_DECL, //contains identifier literal, typenode, expression definition
	NODE_FN_DECL, //containd identifier literal, arguments node, returns node, block node
	NODE_FN_COLLECTION, //parts of a function
	NODE_FN_CALL,
	NODE_PATH_IF, //for control flow
	NODE_PATH_WHILE, //for control flow
	NODE_PATH_FOR, //for control flow
	NODE_PATH_BREAK,
	NODE_PATH_CONTINUE,
	NODE_PATH_RETURN,
	NODE_INCREMENT_PREFIX,
	NODE_INCREMENT_POSTFIX,
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

typedef struct NodeCompound {
	NodeType type;
	LiteralType literalType;
	Node* nodes;
	int capacity;
	int count;
} NodeCompound;

typedef struct NodePair {
	NodeType type;
	Node* left;
	Node* right;
} NodePair;

typedef struct NodeVarDecl {
	NodeType type;
	Literal identifier;
	Literal typeLiteral;
	Node* expression;
} NodeVarDecl;

typedef struct NodeFnDecl {
	NodeType type;
	Literal identifier;
	Node* arguments;
	Node* returns;
	Node* block;
} NodeFnDecl;

typedef struct NodeFnCollection {
	NodeType type;
	Node* nodes;
	int capacity;
	int count;
} NodeFnCollection;

typedef struct NodeFnCall {
	NodeType type;
	Node* arguments;
} NodeFnCall;

typedef struct NodePath {
	NodeType type;
	Node* preClause;
	Node* postClause;
	Node* condition;
	Node* thenPath;
	Node* elsePath;
} NodePath;

typedef struct NodeIncrement {
	NodeType type;
	Literal identifier;
	int increment;
} NodeIncrement;

union _node {
	NodeType type;
	NodeLiteral atomic;
	NodeUnary unary;
	NodeBinary binary;
	NodeGrouping grouping;
	NodeBlock block;
	NodeCompound compound;
	NodePair pair;
	NodeVarDecl varDecl;
	NodeFnDecl fnDecl;
	NodeFnCollection fnCollection;
	NodeFnCall fnCall;
	NodePath path;
	NodeIncrement increment;
};

void freeNode(Node* node);
void emitNodeLiteral(Node** nodeHandle, Literal literal);
void emitNodeUnary(Node** nodeHandle, Opcode opcode, Node* child);
void emitNodeBinary(Node** nodeHandle, Node* rhs, Opcode opcode); //handled node becomes lhs
void emitNodeGrouping(Node** nodeHandle);
void emitNodeBlock(Node** nodeHandle);
void emitNodeCompound(Node** nodeHandle, LiteralType literalType);
void setNodePair(Node* node, Node* left, Node* right);
void emitNodeVarDecl(Node** nodeHandle, Literal identifier, Literal type, Node* expression);
void emitNodeFnDecl(Node** nodeHandle, Literal identifier, Node* arguments, Node* returns, Node* block);
void emitFnCall(Node** nodeHandle, Node* arguments);
void emitNodeFnCollection(Node** nodeHandle);
void emitNodePath(Node** nodeHandle, NodeType type, Node* preClause, Node* postClause, Node* condition, Node* thenPath, Node* elsePath);
void emitNodePrefixIncrement(Node** nodeHandle, Literal identifier, int increment);
void emitNodePostfixIncrement(Node** nodeHandle, Literal identifier, int increment);
