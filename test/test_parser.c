#include "parser.h"

#include "console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//IO functions
char* readFile(char* path, size_t* fileSize) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, ERROR "Could not open file \"%s\"\n" RESET, path);
		exit(-1);
	}

	fseek(file, 0L, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(*fileSize + 1);

	if (buffer == NULL) {
		fprintf(stderr, ERROR "Not enough memory to read \"%s\"\n" RESET, path);
		exit(-1);
	}

	size_t bytesRead = fread(buffer, sizeof(char), *fileSize, file);

	buffer[*fileSize] = '\0'; //NOTE: fread doesn't append this

	if (bytesRead < *fileSize) {
		fprintf(stderr, ERROR "Could not read file \"%s\"\n" RESET, path);
		exit(-1);
	}

	fclose(file);

	return buffer;
}

int main() {
	{
		//source
		char* source = "print null;";

		//test init & quit
		Lexer lexer;
		Parser parser;
		initLexer(&lexer, source);
		initParser(&parser, &lexer);

		freeParser(&parser);
	}

	{
		//source
		char* source = "print null;";

		//test parsing
		Lexer lexer;
		Parser parser;
		initLexer(&lexer, source);
		initParser(&parser, &lexer);

		ASTNode* node = scanParser(&parser);

		//inspect the node
		if (node == NULL) {
			fprintf(stderr, ERROR "ERROR: ASTNode is null" RESET);
			return -1;
		}

		if (node->type != AST_NODE_UNARY || node->unary.opcode != OP_PRINT) {
			fprintf(stderr, ERROR "ERROR: ASTNode is not a unary print instruction" RESET);
			return -1;
		}

		if (node->unary.child->type != AST_NODE_LITERAL || !IS_NULL(node->unary.child->atomic.literal)) {
			fprintf(stderr, ERROR "ERROR: ASTNode to be printed is not a null literal" RESET);
			return -1;
		}

		//cleanup
		freeASTNode(node);
		freeParser(&parser);
	}

	{
		//get the source file
		size_t size = 0;
		char* source = readFile("scripts/sample_code.toy", &size);

		//test parsing a chunk of junk (valgrind will find leaks)
		Lexer lexer;
		Parser parser;
		initLexer(&lexer, source);
		initParser(&parser, &lexer);

		ASTNode* node = scanParser(&parser);

		while (node != NULL) {
			if (node->type == AST_NODE_ERROR) {
				fprintf(stderr, ERROR "ERROR: Error node detected" RESET);
				return -1;
			}

			freeASTNode(node);
			node = scanParser(&parser);
		}

		//cleanup
		freeParser(&parser);
		free((void*)source);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

