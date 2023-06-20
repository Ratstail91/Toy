#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include "toy_console_colors.h"

#include "toy_memory.h"

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
		fprintf(stderr, TOY_CC_ERROR "Assertion failure: ");
		fprintf(stderr, "%s", output);
		fprintf(stderr, "\n" TOY_CC_RESET); //default new line
	}
}

void runBinaryCustom(const unsigned char* tb, size_t size) {
	Toy_Interpreter interpreter;
	Toy_initInterpreter(&interpreter);

	//NOTE: suppress print output for testing
	Toy_setInterpreterPrint(&interpreter, noPrintFn);
	Toy_setInterpreterAssert(&interpreter, noAssertFn);

	Toy_runInterpreter(&interpreter, tb, size);
	Toy_freeInterpreter(&interpreter);
}

void runSourceCustom(const char* source) {
	size_t size = 0;
	const unsigned char* tb = Toy_compileString(source, &size);
	if (!tb) {
		return;
	}
	runBinaryCustom(tb, size);
}

void runSourceFileCustom(const char* fname) {
	size_t size = 0; //not used
	const char* source = (const char*)Toy_readFile(fname, &size);
	runSourceCustom(source);
	free((void*)source);
}

int main() {
	{
		//test init & free
		Toy_Interpreter interpreter;
		Toy_initInterpreter(&interpreter);
		Toy_freeInterpreter(&interpreter);
	}

	{
		//source
		const char* source = "print null;";

		//test basic compilation & collation
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_Compiler compiler;
		Toy_Interpreter interpreter;

		Toy_initLexer(&lexer, source);
		Toy_initParser(&parser, &lexer);
		Toy_initCompiler(&compiler);
		Toy_initInterpreter(&interpreter);

		Toy_ASTNode* node = Toy_scanParser(&parser);

		//write
		Toy_writeCompiler(&compiler, node);

		//collate
		size_t size = 0;
		const unsigned char* bytecode = Toy_collateCompiler(&compiler, &size);

		//NOTE: suppress print output for testing
		Toy_setInterpreterPrint(&interpreter, noPrintFn);
		Toy_setInterpreterAssert(&interpreter, noAssertFn);

		//run
		Toy_runInterpreter(&interpreter, bytecode, size);

		//cleanup
		Toy_freeASTNode(node);
		Toy_freeParser(&parser);
		Toy_freeCompiler(&compiler);
		Toy_freeInterpreter(&interpreter);
	}

	{
		//run each file in tests/scripts/
		const char* filenames[] = {
			"arithmetic.toy",
			"casting-parentheses-bugfix.toy",
			"casting.toy",
			"coercions.toy",
			"comparisons.toy",
			"dot-and-matrix.toy",
			"dot-assignments-bugfix.toy",
			"dot-chaining.toy",
			"dot-modulo-bugfix.toy",
			"dottify-bugfix.toy",
			"function-within-function-bugfix.toy",
			"functions.toy",
			"increment-postfix-bugfix.toy",
			"index-arrays.toy",
			"index-assignment-both-bugfix.toy",
			"index-assignment-intermediate-bugfix.toy",
			"index-assignment-left-bugfix.toy",
			"index-dictionaries.toy",
			"index-strings.toy",
			"jumps.toy",
			"jumps-in-functions.toy",
			"logicals.toy",
			"long-array.toy",
			"long-dictionary.toy",
			"long-literals.toy",
			"native-functions.toy",
			"or-chaining-bugfix.toy",
			"panic-within-functions.toy",
			"polyfill-insert.toy",
			"polyfill-remove.toy",
			"short-circuiting-support.toy",
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
		fprintf(stderr, TOY_CC_ERROR "Assertions hidden: %d\n", ignoredAssertions);
		return -1;
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

