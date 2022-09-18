#include "ast_node.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void freeNodeCustom(ASTNode* node, bool freeSelf) {
	//don't free a NULL node
	if (node == NULL) {
		return;
	}

	switch(node->type) {
		case AST_NODEERROR:
			//NO-OP
		break;

		case AST_NODELITERAL:
			freeLiteral(node->atomic.literal);
		break;

		case AST_NODEUNARY:
			freeNode(node->unary.child);
		break;

		case AST_NODEBINARY:
			freeNode(node->binary.left);
			freeNode(node->binary.right);
		break;

		case AST_NODEGROUPING:
			freeNode(node->grouping.child);
		break;

		case AST_NODEBLOCK:
			for (int i = 0; i < node->block.count; i++) {
				freeNodeCustom(node->block.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->block.nodes, node->block.capacity);
		break;

		case AST_NODECOMPOUND:
			for (int i = 0; i < node->compound.count; i++) {
				freeNodeCustom(node->compound.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->compound.nodes, node->compound.capacity);
		break;

		case AST_NODEPAIR:
			freeNode(node->pair.left);
			freeNode(node->pair.right);
		break;

		case AST_NODEVAR_DECL:
			freeLiteral(node->varDecl.identifier);
			freeLiteral(node->varDecl.typeLiteral);
			freeNode(node->varDecl.expression);
		break;

		case AST_NODEFN_DECL:
			freeLiteral(node->fnDecl.identifier);
			freeNode(node->fnDecl.arguments);
			freeNode(node->fnDecl.returns);
			freeNode(node->fnDecl.block);
		break;

		case AST_NODEFN_COLLECTION:
			for (int i = 0; i < node->fnCollection.count; i++) {
				freeNodeCustom(node->fnCollection.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->fnCollection.nodes, node->fnCollection.capacity);
		break;

		case AST_NODEFN_CALL:
			freeNode(node->fnCall.arguments);
		break;

		case AST_NODEPATH_IF:
		case AST_NODEPATH_WHILE:
		case AST_NODEPATH_FOR:
		case AST_NODEPATH_BREAK:
		case AST_NODEPATH_CONTINUE:
		case AST_NODEPATH_RETURN:
			freeNode(node->path.preClause);
			freeNode(node->path.postClause);
			freeNode(node->path.condition);
			freeNode(node->path.thenPath);
			freeNode(node->path.elsePath);
		break;

		case AST_NODEINCREMENT_PREFIX:
		case AST_NODEINCREMENT_POSTFIX:
			freeLiteral(node->increment.identifier);
		break;

		case AST_NODEIMPORT:
		case AST_NODEEXPORT:
			freeLiteral(node->import.identifier);
			freeLiteral(node->import.alias);
		break;

		case AST_NODEINDEX:
		case AST_NODEDOT:
			freeNode(node->index.first);
			freeNode(node->index.second);
			freeNode(node->index.third);
		break;
	}

	if (freeSelf) {
		FREE(ASTNode, node);
	}
}

void freeNode(ASTNode* node) {
	freeNodeCustom(node, true);
}

void emitASTNodeLiteral(ASTNode** nodeHandle, Literal literal) {
	//allocate a new node
	*nodeHandle = ALLOCATE(ASTNode, 1);

	(*nodeHandle)->type = AST_NODELITERAL;
	(*nodeHandle)->atomic.literal = copyLiteral(literal);
}

void emitASTNodeUnary(ASTNode** nodeHandle, Opcode opcode, ASTNode* child) {
	//allocate a new node
	*nodeHandle = ALLOCATE(ASTNode, 1);

	(*nodeHandle)->type = AST_NODEUNARY;
	(*nodeHandle)->unary.opcode = opcode;
	(*nodeHandle)->unary.child = child;
}

void emitASTNodeBinary(ASTNode** nodeHandle, ASTNode* rhs, Opcode opcode) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEBINARY;
	tmp->binary.opcode = opcode;
	tmp->binary.left = *nodeHandle;
	tmp->binary.right = rhs;

	*nodeHandle = tmp;
}

void emitASTNodeGrouping(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEGROUPING;
	tmp->grouping.child = *nodeHandle;

	*nodeHandle = tmp;
}

void emitASTNodeBlock(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEBLOCK;
	tmp->block.nodes = NULL;
	tmp->block.capacity = 0;
	tmp->block.count = 0;

	*nodeHandle = tmp;
}

void emitASTNodeCompound(ASTNode** nodeHandle, LiteralType literalType) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODECOMPOUND;
	tmp->compound.literalType = literalType;
	tmp->compound.nodes = NULL;
	tmp->compound.capacity = 0;
	tmp->compound.count = 0;

	*nodeHandle = tmp;
}

void setASTNodePair(ASTNode* node, ASTNode* left, ASTNode* right) {
	//assume the node has already been allocated
	node->type = AST_NODEPAIR;
	node->pair.left = left;
	node->pair.right = right;
}

void emitASTNodeVarDecl(ASTNode** nodeHandle, Literal identifier, Literal typeLiteral, ASTNode* expression) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEVAR_DECL;
	tmp->varDecl.identifier = identifier;
	tmp->varDecl.typeLiteral = typeLiteral;
	tmp->varDecl.expression = expression;

	*nodeHandle = tmp;
}

void emitASTNodeFnDecl(ASTNode** nodeHandle, Literal identifier, ASTNode* arguments, ASTNode* returns, ASTNode* block) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEFN_DECL;
	tmp->fnDecl.identifier = identifier;
	tmp->fnDecl.arguments = arguments;
	tmp->fnDecl.returns = returns;
	tmp->fnDecl.block = block;

	*nodeHandle = tmp;
}

void emitASTFnCall(ASTNode** nodeHandle, ASTNode* arguments, int argumentCount) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEFN_CALL;
	tmp->fnCall.arguments = arguments;
	tmp->fnCall.argumentCount = argumentCount;

	*nodeHandle = tmp;
}

void emitASTNodeFnCollection(ASTNode** nodeHandle) { //a collection of nodes, intended for use with functions
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEFN_COLLECTION;
	tmp->fnCollection.nodes = NULL;
	tmp->fnCollection.capacity = 0;
	tmp->fnCollection.count = 0;

	*nodeHandle = tmp;
}

void emitASTNodePath(ASTNode** nodeHandle, ASTNodeType type, ASTNode* preClause, ASTNode* postClause, ASTNode* condition, ASTNode* thenPath, ASTNode* elsePath) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = type;
	tmp->path.preClause = preClause;
	tmp->path.postClause = postClause;
	tmp->path.condition = condition;
	tmp->path.thenPath = thenPath;
	tmp->path.elsePath = elsePath;

	*nodeHandle = tmp;
}

void emitASTNodePrefixIncrement(ASTNode** nodeHandle, Literal identifier, int increment) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEINCREMENT_PREFIX;
	tmp->increment.identifier = copyLiteral(identifier);
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void emitASTNodePostfixIncrement(ASTNode** nodeHandle, Literal identifier, int increment) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEINCREMENT_POSTFIX;
	tmp->increment.identifier = copyLiteral(identifier);
	tmp->increment.increment = increment;

	*nodeHandle = tmp;
}

void emitASTNodeImport(ASTNode** nodeHandle, ASTNodeType mode, Literal identifier, Literal alias) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = mode;
	tmp->import.identifier = copyLiteral(identifier);
	tmp->import.alias = copyLiteral(alias);

	*nodeHandle = tmp;
}

void emitASTNodeIndex(ASTNode** nodeHandle, ASTNode* first, ASTNode* second, ASTNode* third) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEINDEX;
	tmp->index.first = first;
	tmp->index.second = second;
	tmp->index.third = third;

	*nodeHandle = tmp;
}

void emitASTNodeDot(ASTNode** nodeHandle, ASTNode* first) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODEDOT;
	tmp->index.first = first;
	tmp->index.second = NULL;
	tmp->index.third = NULL;

	*nodeHandle = tmp;
}
