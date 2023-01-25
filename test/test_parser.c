#include "toy_parser.h"

#include "toy_console_colors.h"

#include "../repl/repl_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	{
		//source
		char* source = "print null;";

		//test init & quit
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);

		Toy_freeParser(&parser);
	}

	{
		//source
		char* source = "print null;";

		//test parsing
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);

		Toy_ASTNode* node = Toy_scanParser(&parser);

		//inspect the node
		if (node == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: ASTNode is null" TOY_CC_RESET);
			return -1;
		}

		if (node->type != TOY_AST_NODE_UNARY || node->unary.opcode != TOY_OP_PRINT) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: ASTNode is not a unary print instruction" TOY_CC_RESET);
			return -1;
		}

		if (node->unary.child->type != TOY_AST_NODE_LITERAL || !TOY_IS_NULL(node->unary.child->atomic.literal)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: ASTNode to be printed is not a null literal" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeASTNode(node);
		Toy_freeParser(&parser);
	}

	{
		//get the source file
		size_t size = 0;
		char* source = Toy_readFile("scripts/parser_sample_code.toy", &size);

		//test parsing a chunk of junk (valgrind will find leaks)
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);

		Toy_ASTNode* node = Toy_scanParser(&parser);

		while (node != NULL) {
			if (node->type == TOY_AST_NODE_ERROR) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Error node detected" TOY_CC_RESET);
				return -1;
			}

			Toy_freeASTNode(node);
			node = Toy_scanParser(&parser);
		}

		//cleanup
		Toy_freeParser(&parser);
		free((void*)source);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

