#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//suppress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

int errorsTriggered = 0;
static void noErrorFn(const char* output) {
	errorsTriggered++;
}

//compilation functions
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

	//NOTE: suppress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);
	setInterpreterError(&interpreter, noErrorFn);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
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

int main() {
	bool success = true;

	{
		//run each file in tests/scripts/
		char* filenames[] = {
			"declare-types-array.toy",
			"declare-types-dictionary.toy",
			NULL
		};

		for (int i = 0; filenames[i]; i++) {
			printf("Running %s\n", filenames[i]);

			char buffer[128];
			snprintf(buffer, 128, "scripts/mustfail/%s", filenames[i]);

			runSourceFile(buffer);

			if (errorsTriggered == 0) {
				printf(ERROR "Expected error did not occur in %s\n" RESET, filenames[i]);
				success = false;
			}

			errorsTriggered = 0;
		}
	}

	if (!success) {
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

