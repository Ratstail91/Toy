#include "lexer.h"
#include "parser.h"
#include "compiler.h"

#include "console_colors.h"

#include "memory.h"

#include "../repl/repl_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	{
		//test init & free
		Compiler compiler;
		initCompiler(&compiler);
		freeCompiler(&compiler);
	}

	{
		//source
		char* source = "print null;";

		//test basic compilation & collation
		Lexer lexer;
		Parser parser;
		Compiler compiler;

		initLexer(&lexer, source);
		initParser(&parser, &lexer);
		initCompiler(&compiler);

		ASTNode* node = scanParser(&parser);

		//write
		writeCompiler(&compiler, node);

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//cleanup
		FREE_ARRAY(unsigned char, bytecode, size);
		freeASTNode(node);
		freeParser(&parser);
		freeCompiler(&compiler);
	}

	{
		//source
		size_t sourceLength = 0;
		char* source = readFile("scripts/compiler_sample_code.toy", &sourceLength);

		//test basic compilation & collation
		Lexer lexer;
		Parser parser;
		Compiler compiler;

		initLexer(&lexer, source);
		initParser(&parser, &lexer);
		initCompiler(&compiler);

		ASTNode* node = scanParser(&parser);
		while (node != NULL) {
			if (node->type == AST_NODE_ERROR) {
				fprintf(stderr, ERROR "ERROR: Error node found" RESET);
				return -1;
			}

			//write
			writeCompiler(&compiler, node);
			freeASTNode(node);

			node = scanParser(&parser);
		}

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//cleanup
		FREE_ARRAY(char, source, sourceLength);
		FREE_ARRAY(unsigned char, bytecode, size);
		freeParser(&parser);
		freeCompiler(&compiler);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

