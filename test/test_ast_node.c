#include "toy_ast_node.h"

#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//lazy
#define ASSERT(test_for_true) if (!(test_for_true)) {\
	fprintf(stderr, TOY_CC_ERROR "assert failed: %s\n" TOY_CC_RESET, #test_for_true); \
	exit(-1); \
}

int main() {
	//test literals
	{
		//test literals
		char* str = "foobar";
		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefString(str));

		//generate the node
		Toy_ASTNode* node = NULL;
		Toy_emitASTNodeLiteral(&node, literal);

		//check node type
		ASSERT(node->type == TOY_AST_NODE_LITERAL);

		//cleanup
		Toy_freeLiteral(literal);
		Toy_freeASTNode(node);
	}

	//test unary
	{
		//generate the child node
		char* str = "foobar";
		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefString(str));
		Toy_ASTNode* childNode = NULL;
		Toy_emitASTNodeLiteral(&childNode, literal);

		//generate the unary node
		Toy_ASTNode* unary = NULL;
		Toy_emitASTNodeUnary(&unary, TOY_OP_PRINT, childNode);

		//check node type
		ASSERT(unary->type == TOY_AST_NODE_UNARY);

		//cleanup
		Toy_freeLiteral(literal);
		Toy_freeASTNode(unary);
	}

	//test binary
	{
		//generate the child node
		char* str = "foobar";
		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefString(str));
		Toy_ASTNode* nodeHandle = NULL;
		Toy_emitASTNodeLiteral(&nodeHandle, literal);

		Toy_ASTNode* rhsChildNode = NULL;
		Toy_emitASTNodeLiteral(&rhsChildNode, literal);

		//generate the unary node
		Toy_emitASTNodeBinary(&nodeHandle, rhsChildNode, TOY_OP_PRINT);

		//check node type
		ASSERT(nodeHandle->type == TOY_AST_NODE_BINARY);
		ASSERT(nodeHandle->binary.opcode == TOY_OP_PRINT);

		//cleanup
		Toy_freeLiteral(literal);
		Toy_freeASTNode(nodeHandle);
	}

	//TODO: more tests for other AST node types

	//test compounds
	{
		//test compound (dictionary)
		char* idn = "foobar";
		char* str = "hello world";

		Toy_ASTNode* dictionary;
		Toy_ASTNode* left;
		Toy_ASTNode* right;

		Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(idn));
		Toy_Literal string = TOY_TO_STRING_LITERAL(Toy_createRefString(str));

		Toy_emitASTNodeCompound(&dictionary, TOY_LITERAL_DICTIONARY);
		Toy_emitASTNodeLiteral(&left, identifier);
		Toy_emitASTNodeLiteral(&right, string);

		//grow the node if needed
		if (dictionary->compound.capacity < dictionary->compound.count + 1) {
			int oldCapacity = dictionary->compound.capacity;

			dictionary->compound.capacity = TOY_GROW_CAPACITY(oldCapacity);
			dictionary->compound.nodes = TOY_GROW_ARRAY(Toy_ASTNode, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
		}

		//store the left and right in the node
		Toy_setASTNodePair(&dictionary->compound.nodes[dictionary->compound.count++], left, right);

		//the real test
		Toy_freeASTNode(dictionary);
		Toy_freeLiteral(identifier);
		Toy_freeLiteral(string);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

