#include "ast_node.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

void freeASTNodeCustom(ASTNode* node, bool freeSelf) {
	//don't free a NULL node
	if (node == NULL) {
		return;
	}

	switch(node->type) {
		case AST_NODE_ERROR:
			//NO-OP
		break;

		case AST_NODE_LITERAL:
			freeLiteral(node->atomic.literal);
		break;

		case AST_NODE_UNARY:
			freeASTNode(node->unary.child);
		break;

		case AST_NODE_BINARY:
			freeASTNode(node->binary.left);
			freeASTNode(node->binary.right);
		break;

		case AST_NODE_TERNARY:
			freeASTNode(node->ternary.condition);
			freeASTNode(node->ternary.thenPath);
			freeASTNode(node->ternary.elsePath);
		break;

		case AST_NODE_GROUPING:
			freeASTNode(node->grouping.child);
		break;

		case AST_NODE_BLOCK:
			for (int i = 0; i < node->block.count; i++) {
				freeASTNodeCustom(node->block.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->block.nodes, node->block.capacity);
		break;

		case AST_NODE_COMPOUND:
			for (int i = 0; i < node->compound.count; i++) {
				freeASTNodeCustom(node->compound.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->compound.nodes, node->compound.capacity);
		break;

		case AST_NODE_PAIR:
			freeASTNode(node->pair.left);
			freeASTNode(node->pair.right);
		break;

		case AST_NODE_INDEX:
			freeASTNode(node->index.first);
			freeASTNode(node->index.second);
			freeASTNode(node->index.third);
		break;

		case AST_NODE_VAR_DECL:
			freeLiteral(node->varDecl.identifier);
			freeLiteral(node->varDecl.typeLiteral);
			freeASTNode(node->varDecl.expression);
		break;

		case AST_NODE_FN_COLLECTION:
			for (int i = 0; i < node->fnCollection.count; i++) {
				freeASTNodeCustom(node->fnCollection.nodes + i, false);
			}
			FREE_ARRAY(ASTNode, node->fnCollection.nodes, node->fnCollection.capacity);
		break;

		case AST_NODE_FN_DECL:
			freeLiteral(node->fnDecl.identifier);
			freeASTNode(node->fnDecl.arguments);
			freeASTNode(node->fnDecl.returns);
			freeASTNode(node->fnDecl.block);
		break;

		case AST_NODE_FN_CALL:
			freeASTNode(node->fnCall.arguments);
		break;

		case AST_NODE_FN_RETURN:
			freeASTNode(node->returns.returns);
		break;

		case AST_NODE_IF:
			freeASTNode(node->pathIf.condition);
			freeASTNode(node->pathIf.thenPath);
			freeASTNode(node->pathIf.elsePath);
		break;

		case AST_NODE_WHILE:
			freeASTNode(node->pathWhile.condition);
			freeASTNode(node->pathWhile.thenPath);
		break;

		case AST_NODE_FOR:
			freeASTNode(node->pathFor.preClause);
			freeASTNode(node->pathFor.postClause);
			freeASTNode(node->pathFor.condition);
			freeASTNode(node->pathFor.thenPath);
		break;

		case AST_NODE_BREAK:
			//NO-OP
		break;

		case AST_NODE_CONTINUE:
			//NO-OP
		break;

		case AST_NODE_PREFIX_INCREMENT:
			freeLiteral(node->prefixIncrement.identifier);
		break;
		case AST_NODE_PREFIX_DECREMENT:
			freeLiteral(node->prefixDecrement.identifier);
		break;
		case AST_NODE_POSTFIX_INCREMENT:
			freeLiteral(node->postfixIncrement.identifier);
		break;
		case AST_NODE_POSTFIX_DECREMENT:
			freeLiteral(node->postfixDecrement.identifier);
		break;

		case AST_NODE_IMPORT:
			freeLiteral(node->import.identifier);
			freeLiteral(node->import.alias);
		break;
	}

	if (freeSelf) {
		FREE(ASTNode, node);
	}
}

void freeASTNode(ASTNode* node) {
	freeASTNodeCustom(node, true);
}

//various emitters
void emitASTNodeLiteral(ASTNode** nodeHandle, Literal literal) {
	//allocate a new node
	*nodeHandle = ALLOCATE(ASTNode, 1);

	(*nodeHandle)->type = AST_NODE_LITERAL;
	(*nodeHandle)->atomic.literal = copyLiteral(literal);
}

void emitASTNodeUnary(ASTNode** nodeHandle, Opcode opcode, ASTNode* child) {
	//allocate a new node
	*nodeHandle = ALLOCATE(ASTNode, 1);

	(*nodeHandle)->type = AST_NODE_UNARY;
	(*nodeHandle)->unary.opcode = opcode;
	(*nodeHandle)->unary.child = child;
}

void emitASTNodeBinary(ASTNode** nodeHandle, ASTNode* rhs, Opcode opcode) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_BINARY;
	tmp->binary.opcode = opcode;
	tmp->binary.left = *nodeHandle;
	tmp->binary.right = rhs;

	*nodeHandle = tmp;
}

void emitASTNodeTernary(ASTNode** nodeHandle, ASTNode* condition, ASTNode* thenPath, ASTNode* elsePath) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_TERNARY;
	tmp->ternary.condition = condition;
	tmp->ternary.thenPath = thenPath;
	tmp->ternary.elsePath = elsePath;

	*nodeHandle = tmp;
}

void emitASTNodeGrouping(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_GROUPING;
	tmp->grouping.child = *nodeHandle;

	*nodeHandle = tmp;
}

void emitASTNodeBlock(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_BLOCK;
	tmp->block.nodes = NULL; //NOTE: appended by the parser
	tmp->block.capacity = 0;
	tmp->block.count = 0;

	*nodeHandle = tmp;
}

void emitASTNodeCompound(ASTNode** nodeHandle, LiteralType literalType) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_COMPOUND;
	tmp->compound.literalType = literalType;
	tmp->compound.nodes = NULL;
	tmp->compound.capacity = 0;
	tmp->compound.count = 0;

	*nodeHandle = tmp;
}

void setASTNodePair(ASTNode* node, ASTNode* left, ASTNode* right) {
	//set - assume the node has already been allocated
	node->type = AST_NODE_PAIR;
	node->pair.left = left;
	node->pair.right = right;
}

void emitASTNodeIndex(ASTNode** nodeHandle, ASTNode* first, ASTNode* second, ASTNode* third) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_INDEX;
	tmp->index.first = first;
	tmp->index.second = second;
	tmp->index.third = third;

	*nodeHandle = tmp;
}

void emitASTNodeVarDecl(ASTNode** nodeHandle, Literal identifier, Literal typeLiteral, ASTNode* expression) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_VAR_DECL;
	tmp->varDecl.identifier = identifier;
	tmp->varDecl.typeLiteral = typeLiteral;
	tmp->varDecl.expression = expression;

	*nodeHandle = tmp;
}

void emitASTNodeFnCollection(ASTNode** nodeHandle) { //a collection of nodes, intended for use with functions
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_FN_COLLECTION;
	tmp->fnCollection.nodes = NULL;
	tmp->fnCollection.capacity = 0;
	tmp->fnCollection.count = 0;

	*nodeHandle = tmp;
}

void emitASTNodeFnDecl(ASTNode** nodeHandle, Literal identifier, ASTNode* arguments, ASTNode* returns, ASTNode* block) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_FN_DECL;
	tmp->fnDecl.identifier = identifier;
	tmp->fnDecl.arguments = arguments;
	tmp->fnDecl.returns = returns;
	tmp->fnDecl.block = block;

	*nodeHandle = tmp;
}

void emitASTNodeFnCall(ASTNode** nodeHandle, ASTNode* arguments) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_FN_CALL;
	tmp->fnCall.arguments = arguments;
	tmp->fnCall.argumentCount = arguments->fnCollection.count;

	*nodeHandle = tmp;
}

void emitASTNodeFnReturn(ASTNode** nodeHandle, ASTNode* returns) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_FN_RETURN;
	tmp->returns.returns = returns;

	*nodeHandle = tmp;
}

void emitASTNodeIf(ASTNode** nodeHandle, ASTNode* condition, ASTNode* thenPath, ASTNode* elsePath) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_IF;
	tmp->pathIf.condition = condition;
	tmp->pathIf.thenPath = thenPath;
	tmp->pathIf.elsePath = elsePath;

	*nodeHandle = tmp;
}

void emitASTNodeWhile(ASTNode** nodeHandle, ASTNode* condition, ASTNode* thenPath) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_WHILE;
	tmp->pathWhile.condition = condition;
	tmp->pathWhile.thenPath = thenPath;

	*nodeHandle = tmp;
}

void emitASTNodeFor(ASTNode** nodeHandle, ASTNode* preClause, ASTNode* condition, ASTNode* postClause, ASTNode* thenPath) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_FOR;
	tmp->pathFor.preClause = preClause;
	tmp->pathFor.condition = condition;
	tmp->pathFor.postClause = postClause;
	tmp->pathFor.thenPath = thenPath;

	*nodeHandle = tmp;
}

void emitASTNodeBreak(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_BREAK;

	*nodeHandle = tmp;
}

void emitASTNodeContinue(ASTNode** nodeHandle) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_CONTINUE;

	*nodeHandle = tmp;
}

void emitASTNodePrefixIncrement(ASTNode** nodeHandle, Literal identifier) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_PREFIX_INCREMENT;
	tmp->prefixIncrement.identifier = copyLiteral(identifier);

	*nodeHandle = tmp;
}

void emitASTNodePrefixDecrement(ASTNode** nodeHandle, Literal identifier) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_PREFIX_DECREMENT;
	tmp->prefixDecrement.identifier = copyLiteral(identifier);

	*nodeHandle = tmp;
}

void emitASTNodePostfixIncrement(ASTNode** nodeHandle, Literal identifier) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_POSTFIX_INCREMENT;
	tmp->postfixIncrement.identifier = copyLiteral(identifier);

	*nodeHandle = tmp;
}

void emitASTNodePostfixDecrement(ASTNode** nodeHandle, Literal identifier) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_POSTFIX_DECREMENT;
	tmp->postfixDecrement.identifier = copyLiteral(identifier);

	*nodeHandle = tmp;
}

void emitASTNodeImport(ASTNode** nodeHandle, Literal identifier, Literal alias) {
	ASTNode* tmp = ALLOCATE(ASTNode, 1);

	tmp->type = AST_NODE_IMPORT;
	tmp->import.identifier = copyLiteral(identifier);
	tmp->import.alias = copyLiteral(alias);

	*nodeHandle = tmp;
}
