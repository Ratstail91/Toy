#include "node.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void freeNode(Node* node) {
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
				freeNode(node->block.nodes + i);
			}
			FREE_ARRAY(Node, node->block.nodes, node->block.capacity);
		break;

		case NODE_COMPOUND:
			for (int i = 0; i < node->compound.count; i++) {
				freeNode(node->compound.nodes + i);
			}
			FREE_ARRAY(Node, node->compound.nodes, node->compound.capacity);
		break;

		case NODE_PAIR:
			freeNode(node->pair.left);
			freeNode(node->pair.right);
		break;

		case NODE_VAR_TYPES:
			freeLiteral(node->varTypes.typeLiteral);
		break;

		case NODE_VAR_DECL:
			freeLiteral(node->varDecl.identifier);
			freeLiteral(node->varDecl.typeLiteral);
			freeNode(node->varDecl.expression);
		break;

		case NODE_PATH_IF:
		case NODE_PATH_WHILE:
		case NODE_PATH_FOR:
		case NODE_PATH_BREAK:
		case NODE_PATH_CONTINUE:
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
	}

	FREE(Node, node);
}

void emitNodeLiteral(Node** nodeHandle, Literal literal) {
	//allocate a new node
	*nodeHandle = ALLOCATE(Node, 1);

	(*nodeHandle)->type = NODE_LITERAL;
	(*nodeHandle)->atomic.literal = literal;
}

void emitNodeUnary(Node** nodeHandle, Opcode opcode) {
	//allocate a new node
	*nodeHandle = ALLOCATE(Node, 1);

	(*nodeHandle)->type = NODE_UNARY;
	(*nodeHandle)->unary.opcode = opcode;
	(*nodeHandle)->unary.child = NULL;
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
	tmp->grouping.child = NULL;

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

void emitNodePair(Node** nodeHandle, Node* left, Node* right) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_PAIR;
	tmp->pair.left = left;
	tmp->pair.right = right;

	*nodeHandle = tmp;
}

void emitNodeVarTypes(Node** nodeHandle, Literal literal) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_VAR_TYPES;
	tmp->varTypes.typeLiteral = literal;

	*nodeHandle = tmp;
}

void emitNodeVarDecl(Node** nodeHandle, Literal identifier, Literal type, Node* expression) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_VAR_DECL;
	tmp->varDecl.identifier = identifier;
	tmp->varDecl.typeLiteral = type;
	tmp->varDecl.expression = expression;

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

void emiteNodePrefixIncrement(Node** nodeHandle, Literal identifier, int increment) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_INCREMENT_PREFIX;
	tmp->increment.identifier = identifier;
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void emiteNodePostfixIncrement(Node** nodeHandle, Literal identifier, int increment) {
	Node* tmp = ALLOCATE(Node, 1);

	tmp->type = NODE_INCREMENT_POSTFIX;
	tmp->increment.identifier = identifier;
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void printNode(Node* node) {
	if (node == NULL) {
		return;
	}

	switch(node->type) {
		case NODE_ERROR:
			printf("error");
		break;

		case NODE_LITERAL:
			printf("literal:");
			printLiteral(node->atomic.literal);
		break;

		case NODE_UNARY:
			printf("unary:");
			printNode(node->unary.child);
		break;

		case NODE_BINARY:
			printf("binary-left:");
			printNode(node->binary.left);
			printf(";binary-right:");
			printNode(node->binary.right);
			printf(";");
		break;

		case NODE_GROUPING:
			printf("(");
			printNode(node->grouping.child);
			printf(")");
		break;

		case NODE_BLOCK:
			printf("{\n");

			for (int i = 0; i < node->block.count; i++) {
				printNode(&(node->block.nodes[i]));
			}

			printf("\n}\n");
		break;

		case NODE_COMPOUND:
			printf("compound[\n");

			for (int i = 0; i < node->compound.count; i++) {
				printNode(&(node->compound.nodes[i]));
			}

			printf("]\n");
		break;

		case NODE_PAIR:
			printf("pair-left:");
			printNode(node->pair.left);
			printf(";pair-right:");
			printNode(node->pair.right);
			printf(";");
		break;

		case NODE_VAR_TYPES:
			printLiteral(node->varTypes.typeLiteral);
		break;

		case NODE_VAR_DECL:
			printf("vardecl(");
			printLiteral(node->varDecl.identifier);
			printf("; ");
			printLiteral(node->varDecl.typeLiteral);
			printf("; ");
			printNode(node->varDecl.expression);
			printf(")");
		break;

		case NODE_PATH_IF:
		case NODE_PATH_WHILE:
		case NODE_PATH_FOR:
			printf("path(");
			printNode(node->path.preClause);
			printf("; ");
			printNode(node->path.condition);
			printf("; ");
			printNode(node->path.postClause);
			printf("):(");
			printNode(node->path.thenPath);
			printf(")else(");
			printNode(node->path.elsePath);
			printf(")");
		break;

		// case NODE_INCREMENT_PREFIX:
		// case NODE_INCREMENT_POSTFIX:
		// 	//TODO
		// break;

		default:
			printf("[internal] unkown node type in printNode: %d\n", node->type);
	}
}