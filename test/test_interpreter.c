#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

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
		//test init & free
		Interpreter interpreter;
		initInterpreter(&interpreter);
		freeInterpreter(&interpreter);
	}

	{
		//source
		char* source = "print null;";

		//test basic compilation & collation
		Lexer lexer;
		Parser parser;
		Compiler compiler;
		Interpreter interpreter;

		initLexer(&lexer, source);
		initParser(&parser, &lexer);
		initCompiler(&compiler);
		initInterpreter(&interpreter);

		Node* node = scanParser(&parser);

		//write
		writeCompiler(&compiler, node);

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//run
		runInterpreter(&interpreter, bytecode, size);

		//cleanup
		freeNode(node);
		freeParser(&parser);
		freeCompiler(&compiler);
		freeInterpreter(&interpreter);
	}

	{
		//source
		size_t sourceLength = 0;
		char* source = readFile("sample_code.toy", &sourceLength);

		//test basic compilation & collation
		Lexer lexer;
		Parser parser;
		Compiler compiler;
		Interpreter interpreter;

		initLexer(&lexer, source);
		initParser(&parser, &lexer);
		initCompiler(&compiler);
		initInterpreter(&interpreter);

		Node* node = scanParser(&parser);

		//write
		writeCompiler(&compiler, node);

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//run
		runInterpreter(&interpreter, bytecode, size);

		//cleanup
		FREE_ARRAY(char, source, sourceLength);
		freeNode(node);
		freeParser(&parser);
		freeCompiler(&compiler);
		freeInterpreter(&interpreter);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

