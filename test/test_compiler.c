#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"

#include "toy_console_colors.h"

#include "toy_memory.h"

#include "../repl/repl_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	{
		//test init & free
		Toy_Compiler compiler;
		Toy_initCompiler(&compiler);
		Toy_freeCompiler(&compiler);
	}

	{
		//source
		char* source = "print null;";

		//test basic compilation & collation
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_Compiler compiler;

		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);
		Toy_initCompiler(&compiler);

		Toy_ASTNode* node = Toy_scanParser(&parser);

		//write
		Toy_writeCompiler(&compiler, node);

		//collate
		size_t size = 0;
		unsigned char* bytecode = Toy_collateCompiler(&compiler, &size);

		//cleanup
		TOY_FREE_ARRAY(unsigned char, bytecode, size);
		Toy_freeASTNode(node);
		Toy_freeParser(&parser);
		Toy_freeCompiler(&compiler);
	}

	{
		//source
		size_t sourceLength = 0;
		const char* source = (const char*)Toy_readFile("scripts/compiler_sample_code.toy", &sourceLength);

		//test basic compilation & collation
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_Compiler compiler;

		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);
		Toy_initCompiler(&compiler);

		Toy_ASTNode* node = Toy_scanParser(&parser);
		while (node != NULL) {
			if (node->type == TOY_AST_NODE_ERROR) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Error node found" TOY_CC_RESET);
				return -1;
			}

			//write
			Toy_writeCompiler(&compiler, node);
			Toy_freeASTNode(node);

			node = Toy_scanParser(&parser);
		}

		//collate
		size_t size = 0;
		unsigned char* bytecode = Toy_collateCompiler(&compiler, &size);

		//cleanup
		TOY_FREE_ARRAY(char, source, sourceLength);
		TOY_FREE_ARRAY(unsigned char, bytecode, size);
		Toy_freeParser(&parser);
		Toy_freeCompiler(&compiler);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

