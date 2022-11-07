#pragma once

#include "toy_common.h"
#include "literal.h"
#include "opcodes.h"
#include "token_types.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node ASTNode;

typedef enum ASTNodeType {
	AST_NODEERROR,
	AST_NODELITERAL, //a simple value
	AST_NODEUNARY, //one child + opcode
	AST_NODEBINARY, //two children, left and right + opcode
	AST_NODEGROUPING, //one child
	AST_NODEBLOCK, //contains a sub-node array
	AST_NODECOMPOUND, //contains a sub-node array
	AST_NODEPAIR, //contains a left and right
	AST_NODEVAR_DECL, //contains identifier literal, typenode, expression definition
	AST_NODEFN_DECL, //containd identifier literal, arguments node, returns node, block node
	AST_NODEFN_COLLECTION, //parts of a function
	AST_NODEFN_CALL,
	AST_NODEPATH_IF, //for control flow
	AST_NODEPATH_WHILE, //for control flow
	AST_NODEPATH_FOR, //for control flow
	AST_NODEPATH_BREAK, //for control flow
	AST_NODEPATH_CONTINUE, //for control flow
	AST_NODEPATH_RETURN,
	AST_NODEINCREMENT_PREFIX,
	AST_NODEINCREMENT_POSTFIX,
	AST_NODEIMPORT,
	AST_NODEEXPORT,
	AST_NODEINDEX,
	AST_NODEDOT,
} ASTNodeType;

typedef struct NodeLiteral {
	ASTNodeType type;
	Literal literal;
} NodeLiteral;

typedef struct NodeUnary {
	ASTNodeType type;
	Opcode opcode;
	ASTNode* child;
} NodeUnary;

typedef struct NodeBinary {
	ASTNodeType type;
	Opcode opcode;
	ASTNode* left;
	ASTNode* right;
} NodeBinary;

typedef struct NodeGrouping {
	ASTNodeType type;
	ASTNode* child;
} NodeGrouping;

typedef struct NodeBlock {
	ASTNodeType type;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeBlock;

typedef struct NodeCompound {
	ASTNodeType type;
	LiteralType literalType;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeCompound;

typedef struct NodePair {
	ASTNodeType type;
	ASTNode* left;
	ASTNode* right;
} NodePair;

typedef struct NodeVarDecl {
	ASTNodeType type;
	Literal identifier;
	Literal typeLiteral;
	ASTNode* expression;
} NodeVarDecl;

typedef struct NodeFnDecl {
	ASTNodeType type;
	Literal identifier;
	ASTNode* arguments;
	ASTNode* returns;
	ASTNode* block;
} NodeFnDecl;

typedef struct NodeFnCollection {
	ASTNodeType type;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeFnCollection;

typedef struct NodeFnCall {
	ASTNodeType type;
	ASTNode* arguments;
	int argumentCount;
} NodeFnCall;

typedef struct NodePath {
	ASTNodeType type;
	ASTNode* preClause;
	ASTNode* postClause;
	ASTNode* condition;
	ASTNode* thenPath;
	ASTNode* elsePath;
} NodePath;

typedef struct NodeIncrement {
	ASTNodeType type;
	Literal identifier;
	int increment;
} NodeIncrement;

typedef struct NodeImport {
	ASTNodeType type;
	Literal identifier;
	Literal alias;
} NodeImport;

typedef struct NodeIndex {
	ASTNodeType type;
	ASTNode* first;
	ASTNode* second;
	ASTNode* third;
} NodeIndex;

union _node {
	ASTNodeType type;
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
	NodeImport import;
	NodeIndex index;
};

TOY_API void freeNode(ASTNode* node);

void emitASTNodeLiteral(ASTNode** nodeHandle, Literal literal);
void emitASTNodeUnary(ASTNode** nodeHandle, Opcode opcode, ASTNode* child);
void emitASTNodeBinary(ASTNode** nodeHandle, ASTNode* rhs, Opcode opcode); //handled node becomes lhs
void emitASTNodeGrouping(ASTNode** nodeHandle);
void emitASTNodeBlock(ASTNode** nodeHandle);
void emitASTNodeCompound(ASTNode** nodeHandle, LiteralType literalType);
void setASTNodePair(ASTNode* node, ASTNode* left, ASTNode* right);
void emitASTNodeVarDecl(ASTNode** nodeHandle, Literal identifier, Literal type, ASTNode* expression);
void emitASTNodeFnDecl(ASTNode** nodeHandle, Literal identifier, ASTNode* arguments, ASTNode* returns, ASTNode* block);
void emitASTFnCall(ASTNode** nodeHandle, ASTNode* arguments, int argumentCount);
void emitASTNodeFnCollection(ASTNode** nodeHandle);
void emitASTNodePath(ASTNode** nodeHandle, ASTNodeType type, ASTNode* preClause, ASTNode* postClause, ASTNode* condition, ASTNode* thenPath, ASTNode* elsePath);
void emitASTNodePrefixIncrement(ASTNode** nodeHandle, Literal identifier, int increment);
void emitASTNodePostfixIncrement(ASTNode** nodeHandle, Literal identifier, int increment);
void emitASTNodeImport(ASTNode** nodeHandle, ASTNodeType mode, Literal identifier, Literal alias);
void emitASTNodeIndex(ASTNode** nodeHandle, ASTNode* first, ASTNode* second, ASTNode* third);
void emitASTNodeDot(ASTNode** nodeHandle, ASTNode* first);
