#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include "../repl/repl_tools.h"

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

unsigned char* compileStringCustom(char* source, size_t* size) {
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
			errorsTriggered++; //custom error catch
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

void runBinaryCustom(unsigned char* tb, size_t size) {
	Interpreter interpreter;
	initInterpreter(&interpreter);

	//NOTE: suppress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);
	setInterpreterError(&interpreter, noErrorFn);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
}

void runSourceCustom(char* source) {
	size_t size = 0;
	unsigned char* tb = compileStringCustom(source, &size);
	if (!tb) {
		return;
	}
	runBinaryCustom(tb, size);
}

void runSourceFileCustom(char* fname) {
	size_t size = 0; //not used
	char* source = readFile(fname, &size);
	runSourceCustom(source);
	free((void*)source);
}

int main() {
	bool success = true;

	{
		//run each file in tests/scripts/
		char* filenames[] = {
			"declare-types-array.toy",
			"declare-types-dictionary-key.toy",
			"declare-types-dictionary-value.toy",
			"index-arrays-non-integer.toy",
			"unary-inverted-nothing.toy",
			"unary-negative-nothing.toy",
			NULL
		};

		for (int i = 0; filenames[i]; i++) {
			printf("Running (must fail) %s\n", filenames[i]);

			char buffer[128];
			snprintf(buffer, 128, "scripts/mustfail/%s", filenames[i]);

			runSourceFileCustom(buffer);

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

