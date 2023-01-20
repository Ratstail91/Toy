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

int ignoredAssertions = 0;
static void noAssertFn(const char* output) {
	if (strncmp(output, "!ignore", 7) == 0) {
		ignoredAssertions++;
	}
	else {
		fprintf(stderr, ERROR "Assertion failure: ");
		fprintf(stderr, "%s", output);
		fprintf(stderr, "\n" RESET); //default new line
	}
}

void runBinaryCustom(unsigned char* tb, size_t size) {
	Interpreter interpreter;
	initInterpreter(&interpreter);

	//NOTE: suppress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);
	setInterpreterAssert(&interpreter, noAssertFn);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
}

void runSourceCustom(char* source) {
	size_t size = 0;
	unsigned char* tb = compileString(source, &size);
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

		ASTNode* node = scanParser(&parser);

		//write
		writeCompiler(&compiler, node);

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//NOTE: suppress print output for testing
		setInterpreterPrint(&interpreter, noPrintFn);
		setInterpreterAssert(&interpreter, noAssertFn);

		//run
		runInterpreter(&interpreter, bytecode, size);

		//cleanup
		freeASTNode(node);
		freeParser(&parser);
		freeCompiler(&compiler);
		freeInterpreter(&interpreter);
	}

	{
		//run each file in tests/scripts/
		char* filenames[] = {
			"arithmetic.toy",
			"casting.toy",
			"coercions.toy",
			"comparisons.toy",
			"dot-and-matrix.toy",
			"dot-assignments-bugfix.toy",
			"dot-chaining.toy",
			"dottify-bugfix.toy",
			"functions.toy",
			"index-arrays.toy",
			"index-dictionaries.toy",
			"index-strings.toy",
			"jumps.toy",
			"jumps-in-functions.toy",
			"logicals.toy",
			"long-array.toy",
			"long-dictionary.toy",
			"long-literals.toy",
			"native-functions.toy",
			"panic-within-functions.toy",
			"ternary-expressions.toy",
			"types.toy",
			NULL
		};

		for (int i = 0; filenames[i]; i++) {
			printf("Running %s\n", filenames[i]);

			char buffer[128];
			snprintf(buffer, 128, "scripts/%s", filenames[i]);

			runSourceFileCustom(buffer);
		}
	}

	//1, to allow for the assertion test
	if (ignoredAssertions > 1) {
		fprintf(stderr, ERROR "Assertions hidden: %d\n", ignoredAssertions);
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

