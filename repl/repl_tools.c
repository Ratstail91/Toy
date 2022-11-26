#include "repl_tools.h"
#include "lib_standard.h"
#include "lib_timer.h"

#include "console_colors.h"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>

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

void writeFile(char* path, unsigned char* bytes, size_t size) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) {
		fprintf(stderr, ERROR "Could not open file \"%s\"\n" RESET, path);
		exit(-1);
	}

	int written = fwrite(bytes, size, 1, file);

	if (written != 1) {
		fprintf(stderr, ERROR "Could not write file \"%s\"\n" RESET, path);
		exit(-1);
	}

	fclose(file);
}

//repl functions
unsigned char* compileString(char* source, size_t* size) {
	Lexer lexer;
	Parser parser;
	Compiler compiler;

	initLexer(&lexer, source);
	initParser(&parser, &lexer);
	initCompiler(&compiler);

	//run the parser until the end of the source
	ASTNode* node = scanParser(&parser);
	while(node != NULL) {
		//pack up and leave
		if (node->type == AST_NODE_ERROR) {
			printf(ERROR "error node detected\n" RESET);
			freeASTNode(node);
			freeCompiler(&compiler);
			freeParser(&parser);
			return NULL;
		}

		writeCompiler(&compiler, node);
		freeASTNode(node);
		node = scanParser(&parser);
	}

	//get the bytecode dump
	unsigned char* tb = collateCompiler(&compiler, (int*)(size));

	//cleanup
	freeCompiler(&compiler);
	freeParser(&parser);
	//no lexer to clean up

	//finally
	return tb;
}

void runBinary(unsigned char* tb, size_t size) {
	Interpreter interpreter;
	initInterpreter(&interpreter);

	//inject the libs
	injectNativeHook(&interpreter, "standard", hookStandard);
	injectNativeHook(&interpreter, "timer", hookTimer);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
}

void runBinaryFile(char* fname) {
	size_t size = 0; //not used
	unsigned char* tb = (unsigned char*)readFile(fname, &size);
	if (!tb) {
		return;
	}
	runBinary(tb, size);
	//interpreter takes ownership of the binary data
}

void runSource(char* source) {
	size_t size = 0;
	unsigned char* tb = compileString(source, &size);
	if (!tb) {
		return;
	}

	runBinary(tb, size);
}

void runSourceFile(char* fname) {
	size_t size = 0; //not used
	char* source = readFile(fname, &size);
	runSource(source);
	free((void*)source);
}
