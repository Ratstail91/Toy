#include "ast_node.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//lazy
#define ASSERT(test_for_true) if (!(test_for_true)) {\
	fprintf(stderr, ERROR "assert failed: %s\n" RESET, #test_for_true); \
	exit(-1); \
}

int main() {
	//test literals
	{
		//test literals
		char* str = "foobar";
		Literal literal = TO_STRING_LITERAL(createRefString(str));

		//generate the node
		ASTNode* node = NULL;
		emitASTNodeLiteral(&node, literal);

		//check node type
		ASSERT(node->type == AST_NODE_LITERAL);

		//cleanup
		freeLiteral(literal);
		freeASTNode(node);
	}

	//test unary
	{
		//generate the child node
		char* str = "foobar";
		Literal literal = TO_STRING_LITERAL(createRefString(str));
		ASTNode* childNode = NULL;
		emitASTNodeLiteral(&childNode, literal);

		//generate the unary node
		ASTNode* unary = NULL;
		emitASTNodeUnary(&unary, OP_PRINT, childNode);

		//check node type
		ASSERT(unary->type == AST_NODE_UNARY);

		//cleanup
		freeLiteral(literal);
		freeASTNode(unary);
	}

	//test binary
	{
		//generate the child node
		char* str = "foobar";
		Literal literal = TO_STRING_LITERAL(createRefString(str));
		ASTNode* nodeHandle = NULL;
		emitASTNodeLiteral(&nodeHandle, literal);

		ASTNode* rhsChildNode = NULL;
		emitASTNodeLiteral(&rhsChildNode, literal);

		//generate the unary node
		emitASTNodeBinary(&nodeHandle, rhsChildNode, OP_PRINT);

		//check node type
		ASSERT(nodeHandle->type == AST_NODE_BINARY);
		ASSERT(nodeHandle->binary.opcode == OP_PRINT);

		//cleanup
		freeLiteral(literal);
		freeASTNode(nodeHandle);
	}

	//TODO: more tests for other AST node types

	//test compounds
	{
		//test compound (dictionary)
		char* idn = "foobar";
		char* str = "hello world";

		ASTNode* dictionary;
		ASTNode* left;
		ASTNode* right;

		Literal identifier = TO_IDENTIFIER_LITERAL(createRefString(idn));
		Literal string = TO_STRING_LITERAL(createRefString(str));

		emitASTNodeCompound(&dictionary, LITERAL_DICTIONARY);
		emitASTNodeLiteral(&left, identifier);
		emitASTNodeLiteral(&right, string);

		//grow the node if needed
		if (dictionary->compound.capacity < dictionary->compound.count + 1) {
			int oldCapacity = dictionary->compound.capacity;

			dictionary->compound.capacity = GROW_CAPACITY(oldCapacity);
			dictionary->compound.nodes = GROW_ARRAY(ASTNode, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
		}

		//store the left and right in the node
		setASTNodePair(&dictionary->compound.nodes[dictionary->compound.count++], left, right);

		//the real test
		freeASTNode(dictionary);
		freeLiteral(identifier);
		freeLiteral(string);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

