#pragma once

#include "toy_common.h"
#include "literal.h"
#include "opcodes.h"
#include "token_types.h"

//nodes are the intermediaries between parsers and compilers
typedef union _node ASTNode;

typedef enum ASTNodeType {
	AST_NODE_ERROR,
	AST_NODE_LITERAL, //a simple value
	AST_NODE_UNARY, //one child + opcode
	AST_NODE_BINARY, //two children, left and right + opcode
	AST_NODE_GROUPING, //one child
	AST_NODE_BLOCK, //contains a sub-node array
	AST_NODE_COMPOUND, //contains a sub-node array
	AST_NODE_PAIR, //contains a left and right
	AST_NODE_INDEX, //index a variable
	AST_NODE_VAR_DECL, //contains identifier literal, typenode, expression definition
	AST_NODE_FN_DECL, //containd identifier literal, arguments node, returns node, block node
	AST_NODE_FN_COLLECTION, //parts of a function
	AST_NODE_FN_CALL, //call a function
	AST_NODE_FN_RETURN, //for control flow
	AST_NODE_IF, //for control flow
	AST_NODE_WHILE, //for control flow
	AST_NODE_FOR, //for control flow
	AST_NODE_BREAK, //for control flow
	AST_NODE_CONTINUE, //for control flow
	AST_NODE_PREFIX_INCREMENT, //increment a variable
	AST_NODE_POSTFIX_INCREMENT, //increment a variable
	AST_NODE_PREFIX_DECREMENT, //decrement a variable
	AST_NODE_POSTFIX_DECREMENT, //decrement a variable
	AST_NODE_IMPORT, //import a variable
	AST_NODE_EXPORT, //export a variable
} ASTNodeType;

//literals
void emitASTNodeLiteral(ASTNode** nodeHandle, Literal literal);

typedef struct NodeLiteral {
	ASTNodeType type;
	Literal literal;
} NodeLiteral;

//unary operator
void emitASTNodeUnary(ASTNode** nodeHandle, Opcode opcode, ASTNode* child);

typedef struct NodeUnary {
	ASTNodeType type;
	Opcode opcode;
	ASTNode* child;
} NodeUnary;

//binary operator
void emitASTNodeBinary(ASTNode** nodeHandle, ASTNode* rhs, Opcode opcode); //handled node becomes lhs

typedef struct NodeBinary {
	ASTNodeType type;
	Opcode opcode;
	ASTNode* left;
	ASTNode* right;
} NodeBinary;

//grouping of other AST nodes
void emitASTNodeGrouping(ASTNode** nodeHandle);

typedef struct NodeGrouping {
	ASTNodeType type;
	ASTNode* child;
} NodeGrouping;

//block of statement nodes
void emitASTNodeBlock(ASTNode** nodeHandle);

typedef struct NodeBlock {
	ASTNodeType type;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeBlock;

//compound literals (array, dictionary)
void emitASTNodeCompound(ASTNode** nodeHandle, LiteralType literalType);

typedef struct NodeCompound {
	ASTNodeType type;
	LiteralType literalType;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeCompound;

void setASTNodePair(ASTNode* node, ASTNode* left, ASTNode* right); //NOTE: this is a set function, not an emit function

typedef struct NodePair {
	ASTNodeType type;
	ASTNode* left;
	ASTNode* right;
} NodePair;

void emitASTNodeIndex(ASTNode** nodeHandle, ASTNode* first, ASTNode* second, ASTNode* third);

typedef struct NodeIndex {
	ASTNodeType type;
	ASTNode* first;
	ASTNode* second;
	ASTNode* third;
} NodeIndex;

//variable declaration
void emitASTNodeVarDecl(ASTNode** nodeHandle, Literal identifier, Literal type, ASTNode* expression);

typedef struct NodeVarDecl {
	ASTNodeType type;
	Literal identifier;
	Literal typeLiteral;
	ASTNode* expression;
} NodeVarDecl;

//NOTE: fnCollection is used by fnDecl, fnCall and fnReturn
void emitASTNodeFnCollection(ASTNode** nodeHandle);

typedef struct NodeFnCollection {
	ASTNodeType type;
	ASTNode* nodes;
	int capacity;
	int count;
} NodeFnCollection;

//function declaration
void emitASTNodeFnDecl(ASTNode** nodeHandle, Literal identifier, ASTNode* arguments, ASTNode* returns, ASTNode* block);

typedef struct NodeFnDecl {
	ASTNodeType type;
	Literal identifier;
	ASTNode* arguments;
	ASTNode* returns;
	ASTNode* block;
} NodeFnDecl;

//function call
void emitASTNodeFnCall(ASTNode** nodeHandle, ASTNode* arguments);

typedef struct NodeFnCall {
	ASTNodeType type;
	ASTNode* arguments;
	int argumentCount; //NOTE: leave this, so it can be hacked by dottify()
} NodeFnCall;

//function return
void emitASTNodeFnReturn(ASTNode** nodeHandle, ASTNode* returns);

typedef struct NodeFnReturn {
	ASTNodeType type;
	ASTNode* returns;
} NodeFnReturn;

//control flow path - if-else, while, for, break, continue, return
void emitASTNodeIf(ASTNode** nodeHandle, ASTNode* condition, ASTNode* thenPath, ASTNode* elsePath);
void emitASTNodeWhile(ASTNode** nodeHandle, ASTNode* condition, ASTNode* thenPath);
void emitASTNodeFor(ASTNode** nodeHandle, ASTNode* preClause, ASTNode* condition, ASTNode* postClause, ASTNode* thenPath);
void emitASTNodeBreak(ASTNode** nodeHandle);
void emitASTNodeContinue(ASTNode** nodeHandle);

typedef struct NodeIf {
	ASTNodeType type;
	ASTNode* condition;
	ASTNode* thenPath;
	ASTNode* elsePath;
} NodeIf;

typedef struct NodeWhile {
	ASTNodeType type;
	ASTNode* condition;
	ASTNode* thenPath;
} NodeWhile;

typedef struct NodeFor {
	ASTNodeType type;
	ASTNode* preClause;
	ASTNode* condition;
	ASTNode* postClause;
	ASTNode* thenPath;
} NodeFor;

typedef struct NodeBreak {
	ASTNodeType type;
} NodeBreak;

typedef struct NodeContinue {
	ASTNodeType type;
} NodeContinue;

//pre-post increment/decrement
void emitASTNodePrefixIncrement(ASTNode** nodeHandle, Literal identifier);
void emitASTNodePrefixDecrement(ASTNode** nodeHandle, Literal identifier);
void emitASTNodePostfixIncrement(ASTNode** nodeHandle, Literal identifier);
void emitASTNodePostfixDecrement(ASTNode** nodeHandle, Literal identifier);

typedef struct NodePrefixIncrement {
	ASTNodeType type;
	Literal identifier;
} NodePrefixIncrement;

typedef struct NodePrefixDecrement {
	ASTNodeType type;
	Literal identifier;
} NodePrefixDecrement;

typedef struct NodePostfixIncrement {
	ASTNodeType type;
	Literal identifier;
} NodePostfixIncrement;

typedef struct NodePostfixDecrement {
	ASTNodeType type;
	Literal identifier;
} NodePostfixDecrement;

//import/export a variable
void emitASTNodeImport(ASTNode** nodeHandle, Literal identifier, Literal alias);
void emitASTNodeExport(ASTNode** nodeHandle, Literal identifier, Literal alias);

typedef struct NodeImport {
	ASTNodeType type;
	Literal identifier;
	Literal alias;
} NodeImport;

typedef struct NodeExport {
	ASTNodeType type;
	Literal identifier;
	Literal alias;
} NodeExport;

union _node {
	ASTNodeType type;
	NodeLiteral atomic;
	NodeUnary unary;
	NodeBinary binary;
	NodeGrouping grouping;
	NodeBlock block;
	NodeCompound compound;
	NodePair pair;
	NodeIndex index;
	NodeVarDecl varDecl;
	NodeFnCollection fnCollection;
	NodeFnDecl fnDecl;
	NodeFnCall fnCall;
	NodeFnReturn returns;
	NodeIf pathIf; //TODO: rename these to ifStmt?
	NodeWhile pathWhile;
	NodeFor pathFor;
	NodeBreak pathBreak;
	NodeContinue pathContinue;
	NodePrefixIncrement prefixIncrement;
	NodePrefixDecrement prefixDecrement;
	NodePostfixIncrement postfixIncrement;
	NodePostfixDecrement postfixDecrement;
	NodeImport import;
	NodeExport export;
};

TOY_API void freeASTNode(ASTNode* node);
