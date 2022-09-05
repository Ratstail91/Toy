#include "node.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void freeNodeCustom(Node* node, bool freeSelf) {
	//don't free a NULL node
	if (node == NULL) {
		return;
	}

	switch(node->type) {
		case NODE_ERROR:
			//NO-OP
		break;

		case NODE_LITERAL:
			freeLiteral(node->atomic.literal);
		break;

		case NODE_UNARY:
			freeNode(node->unary.child);
		break;

		case NODE_BINARY:
			freeNode(node->binary.left);
			freeNode(node->binary.right);
		break;

		case NODE_GROUPING:
			freeNode(node->grouping.child);
		break;

		case NODE_BLOCK:
			for (int i = 0; i < node->block.count; i++) {
				freeNodeCustom(node->block.nodes + i, false);
			}
			FREE_ARRAY(Node, node->block.nodes, node->block.capacity);
		break;

		case NODE_COMPOUND:
			for (int i = 0; i < node->compound.count; i++) {
				freeNodeCustom(node->compound.nodes + i, false);
			}
			FREE_ARRAY(Node, node->compound.nodes, node->compound.capacity);
		break;

		case NODE_PAIR:
			freeNode(node->pair.left);
			freeNode(node->pair.right);
		break;

		case NODE_VAR_DECL:
			freeLiteral(node->varDecl.identifier);
			freeLiteral(node->varDecl.typeLiteral);
			freeNode(node->varDecl.expression);
		break;

		case NODE_FN_DECL:
			freeLiteral(node->fnDecl.identifier);
			freeNode(node->fnDecl.arguments);
			freeNode(node->fnDecl.returns);
			freeNode(node->fnDecl.block);
		break;

		case NODE_FN_COLLECTION:
			for (int i = 0; i < node->fnCollection.count; i++) {
				freeNodeCustom(node->fnCollection.nodes + i, false);
			}
			FREE_ARRAY(Node, node->fnCollection.nodes, node->fnCollection.capacity);
		break;

		case NODE_FN_CALL:
			freeNode(node->fnCall.arguments);
		break;

		case NODE_PATH_IF:
		case NODE_PATH_WHILE:
		case NODE_PATH_FOR:
		case NODE_PATH_BREAK:
		case NODE_PATH_CONTINUE:
		case NODE_PATH_RETURN:
			freeNode(node->path.preClause);
			freeNode(node->path.postClause);
			freeNode(node->path.condition);
			freeNode(node->path.thenPath);
			freeNode(node->path.elsePath);
		break;

		case NODE_INCREMENT_PREFIX:
		case NODE_INCREMENT_POSTFIX:
			freeLiteral(node->increment.identifier);
		break;

		case NODE_IMPORT:
		case NODE_EXPORT:
			freeLiteral(node->import.identifier);
			freeLiteral(node->import.alias);
		break;
	}

	if (freeSelf) {
		FREE(Node, node);
	}
}

void freeNode(Node* node) {
	freeNodeCustom(node, true);
}

void emitNodeLiteral(Node** nodeHandle, Literal literal) {
	//allocate a new node
	*nodeHandle = ALLOCATE(Node, 1);

	(*nodeHandle)->type = NODE_LITERAL;
	(*nodeHandle)->atomic.literal = copyLiteral(literal);
}

void emitNodeUnary(Node** nodeHandle, Opcode opcode, Node* child) {
	//allocate a new node
	*nodeHandle = ALLOCATE(Node, 1);

	(*nodeHandle)->type = NODE_UNARY;
	(*nodeHandle)->unary.opcode = opcode;
	(*nodeHandle)->unary.child = child;
}

void emitNodeBinary(Node** nodeHandle, Node* rhs, Opcode opcode) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_BINARY;
	tmp->binary.opcode = opcode;
	tmp->binary.left = *nodeHandle;
	tmp->binary.right = rhs;

	*nodeHandle = tmp;
}

void emitNodeGrouping(Node** nodeHandle) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_GROUPING;
	tmp->grouping.child = *nodeHandle;

	*nodeHandle = tmp;
}

void emitNodeBlock(Node** nodeHandle) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_BLOCK;
	tmp->block.nodes = NULL;
	tmp->block.capacity = 0;
	tmp->block.count = 0;

	*nodeHandle = tmp;
}

void emitNodeCompound(Node** nodeHandle, LiteralType literalType) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_COMPOUND;
	tmp->compound.literalType = literalType;
	tmp->compound.nodes = NULL;
	tmp->compound.capacity = 0;
	tmp->compound.count = 0;

	*nodeHandle = tmp;
}

void setNodePair(Node* node, Node* left, Node* right) {
	//assume the node has already been allocated
	node->type = NODE_PAIR;
	node->pair.left = left;
	node->pair.right = right;
}

void emitNodeVarDecl(Node** nodeHandle, Literal identifier, Literal typeLiteral, Node* expression) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_VAR_DECL;
	tmp->varDecl.identifier = identifier;
	tmp->varDecl.typeLiteral = typeLiteral;
	tmp->varDecl.expression = expression;

	*nodeHandle = tmp;
}

void emitNodeFnDecl(Node** nodeHandle, Literal identifier, Node* arguments, Node* returns, Node* block) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_FN_DECL;
	tmp->fnDecl.identifier = identifier;
	tmp->fnDecl.arguments = arguments;
	tmp->fnDecl.returns = returns;
	tmp->fnDecl.block = block;

	*nodeHandle = tmp;
}

void emitFnCall(Node** nodeHandle, Node* arguments) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_FN_CALL;
	tmp->fnCall.arguments = arguments;

	*nodeHandle = tmp;
}

void emitNodeFnCollection(Node** nodeHandle) { //a collection of nodes, intended for use with functions
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_FN_COLLECTION;
	tmp->fnCollection.nodes = NULL;
	tmp->fnCollection.capacity = 0;
	tmp->fnCollection.count = 0;

	*nodeHandle = tmp;
}

void emitNodePath(Node** nodeHandle, NodeType type, Node* preClause, Node* postClause, Node* condition, Node* thenPath, Node* elsePath) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = type;
	tmp->path.preClause = preClause;
	tmp->path.postClause = postClause;
	tmp->path.condition = condition;
	tmp->path.thenPath = thenPath;
	tmp->path.elsePath = elsePath;

	*nodeHandle = tmp;
}

void emitNodePrefixIncrement(Node** nodeHandle, Literal identifier, int increment) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_INCREMENT_PREFIX;
	tmp->increment.identifier = copyLiteral(identifier);
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void emitNodePostfixIncrement(Node** nodeHandle, Literal identifier, int increment) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_INCREMENT_POSTFIX;
	tmp->increment.identifier = copyLiteral(identifier);
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void emitNodeImport(Node** nodeHandle, NodeType mode, Literal identifier, Literal alias) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = mode;
	tmp->import.identifier = copyLiteral(identifier);
	tmp->import.alias = copyLiteral(alias);

	*nodeHandle = tmp;
}