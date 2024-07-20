#include "toy_ast_node.h"

#include "toy_memory.h"

#include <stdio.h>
#include <stdlib.h>

static void freeASTNodeCustom(Toy_ASTNode* node, bool freeSelf) {
	//don't free a NULL node
	if (node == NULL) {
		return;
	}

	switch(node->type) {
		case TOY_AST_NODE_ERROR:
			//NO-OP
		break;

		case TOY_AST_NODE_LITERAL:
			Toy_freeLiteral(node->atomic.literal);
		break;

		case TOY_AST_NODE_UNARY:
			Toy_freeASTNode(node->unary.child);
		break;

		case TOY_AST_NODE_BINARY:
			Toy_freeASTNode(node->binary.left);
			Toy_freeASTNode(node->binary.right);
		break;

		case TOY_AST_NODE_TERNARY:
			Toy_freeASTNode(node->ternary.condition);
			Toy_freeASTNode(node->ternary.thenPath);
			Toy_freeASTNode(node->ternary.elsePath);
		break;

		case TOY_AST_NODE_GROUPING:
			Toy_freeASTNode(node->grouping.child);
		break;

		case TOY_AST_NODE_BLOCK:
			if (node->block.capacity > 0) {
				for (int i = 0; i < node->block.count; i++) {
					freeASTNodeCustom(node->block.nodes + i, false);
				}
				TOY_FREE_ARRAY(Toy_ASTNode, node->block.nodes, node->block.capacity);
			}
		break;

		case TOY_AST_NODE_COMPOUND:
			if (node->compound.capacity > 0) {
				for (int i = 0; i < node->compound.count; i++) {
					freeASTNodeCustom(node->compound.nodes + i, false);
				}
				TOY_FREE_ARRAY(Toy_ASTNode, node->compound.nodes, node->compound.capacity);
			}
		break;

		case TOY_AST_NODE_PAIR:
			Toy_freeASTNode(node->pair.left);
			Toy_freeASTNode(node->pair.right);
		break;

		case TOY_AST_NODE_INDEX:
			Toy_freeASTNode(node->index.first);
			Toy_freeASTNode(node->index.second);
			Toy_freeASTNode(node->index.third);
		break;

		case TOY_AST_NODE_VAR_DECL:
			Toy_freeLiteral(node->varDecl.identifier);
			Toy_freeLiteral(node->varDecl.typeLiteral);
			Toy_freeASTNode(node->varDecl.expression);
		break;

		case TOY_AST_NODE_FN_COLLECTION:
			if (node->fnCollection.capacity > 0) {
				for (int i = 0; i < node->fnCollection.count; i++) {
					freeASTNodeCustom(node->fnCollection.nodes + i, false);
				}
				TOY_FREE_ARRAY(Toy_ASTNode, node->fnCollection.nodes, node->fnCollection.capacity);
			}
		break;

		case TOY_AST_NODE_FN_DECL:
			Toy_freeLiteral(node->fnDecl.identifier);
			Toy_freeASTNode(node->fnDecl.arguments);
			Toy_freeASTNode(node->fnDecl.returns);
			Toy_freeASTNode(node->fnDecl.block);
		break;

		case TOY_AST_NODE_FN_CALL:
			Toy_freeASTNode(node->fnCall.arguments);
		break;

		case TOY_AST_NODE_FN_RETURN:
			Toy_freeASTNode(node->returns.returns);
		break;

		case TOY_AST_NODE_IF:
			Toy_freeASTNode(node->pathIf.condition);
			Toy_freeASTNode(node->pathIf.thenPath);
			Toy_freeASTNode(node->pathIf.elsePath);
		break;

		case TOY_AST_NODE_WHILE:
			Toy_freeASTNode(node->pathWhile.condition);
			Toy_freeASTNode(node->pathWhile.thenPath);
		break;

		case TOY_AST_NODE_FOR:
			Toy_freeASTNode(node->pathFor.preClause);
			Toy_freeASTNode(node->pathFor.postClause);
			Toy_freeASTNode(node->pathFor.condition);
			Toy_freeASTNode(node->pathFor.thenPath);
		break;

		case TOY_AST_NODE_BREAK:
			//NO-OP
		break;

		case TOY_AST_NODE_CONTINUE:
			//NO-OP
		break;

		case TOY_AST_NODE_AND:
			Toy_freeASTNode(node->pathAnd.left);
			Toy_freeASTNode(node->pathAnd.right);
		break;

		case TOY_AST_NODE_OR:
			Toy_freeASTNode(node->pathOr.left);
			Toy_freeASTNode(node->pathOr.right);
		break;

		case TOY_AST_NODE_PREFIX_INCREMENT:
			Toy_freeLiteral(node->prefixIncrement.identifier);
		break;
		case TOY_AST_NODE_PREFIX_DECREMENT:
			Toy_freeLiteral(node->prefixDecrement.identifier);
		break;
		case TOY_AST_NODE_POSTFIX_INCREMENT:
			Toy_freeLiteral(node->postfixIncrement.identifier);
		break;
		case TOY_AST_NODE_POSTFIX_DECREMENT:
			Toy_freeLiteral(node->postfixDecrement.identifier);
		break;

		case TOY_AST_NODE_IMPORT:
			Toy_freeLiteral(node->import.identifier);
			Toy_freeLiteral(node->import.alias);
		break;

		case TOY_AST_NODE_PASS:
			//EMPTY
		break;
	}

	if (freeSelf) {
		TOY_FREE(Toy_ASTNode, node);
	}
}

void Toy_freeASTNode(Toy_ASTNode* node) {
	freeASTNodeCustom(node, true);
}

//various emitters
void Toy_emitASTNodeLiteral(Toy_ASTNode** nodeHandle, Toy_Literal literal) {
	//allocate a new node
	*nodeHandle = TOY_ALLOCATE(Toy_ASTNode, 1);

	(*nodeHandle)->type = TOY_AST_NODE_LITERAL;
	(*nodeHandle)->atomic.literal = Toy_copyLiteral(literal);
}

void Toy_emitASTNodeUnary(Toy_ASTNode** nodeHandle, Toy_Opcode opcode, Toy_ASTNode* child) {
	//allocate a new node
	*nodeHandle = TOY_ALLOCATE(Toy_ASTNode, 1);

	(*nodeHandle)->type = TOY_AST_NODE_UNARY;
	(*nodeHandle)->unary.opcode = opcode;
	(*nodeHandle)->unary.child = child;
}

void Toy_emitASTNodeBinary(Toy_ASTNode** nodeHandle, Toy_ASTNode* rhs, Toy_Opcode opcode) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_BINARY;
	tmp->binary.opcode = opcode;
	tmp->binary.left = *nodeHandle;
	tmp->binary.right = rhs;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeTernary(Toy_ASTNode** nodeHandle, Toy_ASTNode* condition, Toy_ASTNode* thenPath, Toy_ASTNode* elsePath) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_TERNARY;
	tmp->ternary.condition = condition;
	tmp->ternary.thenPath = thenPath;
	tmp->ternary.elsePath = elsePath;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeGrouping(Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_GROUPING;
	tmp->grouping.child = *nodeHandle;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeBlock(Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_BLOCK;
	tmp->block.nodes = NULL; //NOTE: appended by the parser
	tmp->block.capacity = 0;
	tmp->block.count = 0;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeCompound(Toy_ASTNode** nodeHandle, Toy_LiteralType literalType) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_COMPOUND;
	tmp->compound.literalType = literalType;
	tmp->compound.nodes = NULL;
	tmp->compound.capacity = 0;
	tmp->compound.count = 0;

	*nodeHandle = tmp;
}

void Toy_setASTNodePair(Toy_ASTNode* node, Toy_ASTNode* left, Toy_ASTNode* right) {
	//set - assume the node has already been allocated
	node->type = TOY_AST_NODE_PAIR;
	node->pair.left = left;
	node->pair.right = right;
}

void Toy_emitASTNodeIndex(Toy_ASTNode** nodeHandle, Toy_ASTNode* first, Toy_ASTNode* second, Toy_ASTNode* third) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_INDEX;
	tmp->index.first = first;
	tmp->index.second = second;
	tmp->index.third = third;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeVarDecl(Toy_ASTNode** nodeHandle, Toy_Literal identifier, Toy_Literal typeLiteral, Toy_ASTNode* expression) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_VAR_DECL;
	tmp->varDecl.identifier = identifier;
	tmp->varDecl.typeLiteral = typeLiteral;
	tmp->varDecl.expression = expression;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeFnCollection(Toy_ASTNode** nodeHandle) { //a collection of nodes, intended for use with functions
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_FN_COLLECTION;
	tmp->fnCollection.nodes = NULL;
	tmp->fnCollection.capacity = 0;
	tmp->fnCollection.count = 0;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeFnDecl(Toy_ASTNode** nodeHandle, Toy_Literal identifier, Toy_ASTNode* arguments, Toy_ASTNode* returns, Toy_ASTNode* block) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_FN_DECL;
	tmp->fnDecl.identifier = identifier;
	tmp->fnDecl.arguments = arguments;
	tmp->fnDecl.returns = returns;
	tmp->fnDecl.block = block;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeFnCall(Toy_ASTNode** nodeHandle, Toy_ASTNode* arguments) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_FN_CALL;
	tmp->fnCall.arguments = arguments;
	tmp->fnCall.argumentCount = arguments->fnCollection.count;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeFnReturn(Toy_ASTNode** nodeHandle, Toy_ASTNode* returns) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_FN_RETURN;
	tmp->returns.returns = returns;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeIf(Toy_ASTNode** nodeHandle, Toy_ASTNode* condition, Toy_ASTNode* thenPath, Toy_ASTNode* elsePath) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_IF;
	tmp->pathIf.condition = condition;
	tmp->pathIf.thenPath = thenPath;
	tmp->pathIf.elsePath = elsePath;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeWhile(Toy_ASTNode** nodeHandle, Toy_ASTNode* condition, Toy_ASTNode* thenPath) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_WHILE;
	tmp->pathWhile.condition = condition;
	tmp->pathWhile.thenPath = thenPath;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeFor(Toy_ASTNode** nodeHandle, Toy_ASTNode* preClause, Toy_ASTNode* condition, Toy_ASTNode* postClause, Toy_ASTNode* thenPath) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_FOR;
	tmp->pathFor.preClause = preClause;
	tmp->pathFor.condition = condition;
	tmp->pathFor.postClause = postClause;
	tmp->pathFor.thenPath = thenPath;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeBreak(Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_BREAK;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeContinue(Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_CONTINUE;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeAnd(Toy_ASTNode** nodeHandle, Toy_ASTNode* rhs) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_AND;
	tmp->pathAnd.left = *nodeHandle;
	tmp->pathAnd.right = rhs;

	*nodeHandle = tmp;
}

void Toy_emitASTNodeOr(Toy_ASTNode** nodeHandle, Toy_ASTNode* rhs) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_OR;
	tmp->pathOr.left = *nodeHandle;
	tmp->pathOr.right = rhs;

	*nodeHandle = tmp;
}

void Toy_emitASTNodePrefixIncrement(Toy_ASTNode** nodeHandle, Toy_Literal identifier) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_PREFIX_INCREMENT;
	tmp->prefixIncrement.identifier = Toy_copyLiteral(identifier);

	*nodeHandle = tmp;
}

void Toy_emitASTNodePrefixDecrement(Toy_ASTNode** nodeHandle, Toy_Literal identifier) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_PREFIX_DECREMENT;
	tmp->prefixDecrement.identifier = Toy_copyLiteral(identifier);

	*nodeHandle = tmp;
}

void Toy_emitASTNodePostfixIncrement(Toy_ASTNode** nodeHandle, Toy_Literal identifier) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_POSTFIX_INCREMENT;
	tmp->postfixIncrement.identifier = Toy_copyLiteral(identifier);

	*nodeHandle = tmp;
}

void Toy_emitASTNodePostfixDecrement(Toy_ASTNode** nodeHandle, Toy_Literal identifier) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_POSTFIX_DECREMENT;
	tmp->postfixDecrement.identifier = Toy_copyLiteral(identifier);

	*nodeHandle = tmp;
}

void Toy_emitASTNodeImport(Toy_ASTNode** nodeHandle, Toy_Literal identifier, Toy_Literal alias) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_IMPORT;
	tmp->import.identifier = Toy_copyLiteral(identifier);
	tmp->import.alias = Toy_copyLiteral(alias);

	*nodeHandle = tmp;
}

void Toy_emitASTNodePass(Toy_ASTNode** nodeHandle) {
	Toy_ASTNode* tmp = TOY_ALLOCATE(Toy_ASTNode, 1);

	tmp->type = TOY_AST_NODE_PASS;

	*nodeHandle = tmp;
}