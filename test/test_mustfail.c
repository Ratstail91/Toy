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

int errorsTriggered = 0;
static void noErrorFn(const char* output) {
	errorsTriggered++;
}

const unsigned char* compileStringCustom(const char* source, size_t* size) {
	Toy_Lexer lexer;
	Toy_Parser parser;
	Toy_Compiler compiler;

	Toy_initLexer(&lexer, source);
	Toy_initParser(&parser, &lexer);
	Toy_initCompiler(&compiler);

	//run the parser until the end of the source
	Toy_ASTNode* node = Toy_scanParser(&parser);
	while(node != NULL) {
		//pack up and leave
		if (node->type == TOY_AST_NODE_ERROR) {
			errorsTriggered++; //custom error catch
			Toy_freeASTNode(node);
			Toy_freeCompiler(&compiler);
			Toy_freeParser(&parser);
			return NULL;
		}

		Toy_writeCompiler(&compiler, node);
		Toy_freeASTNode(node);
		node = Toy_scanParser(&parser);
	}

	//get the bytecode dump
	const unsigned char* tb = Toy_collateCompiler(&compiler, size);

	//cleanup
	Toy_freeCompiler(&compiler);
	Toy_freeParser(&parser);
	//no lexer to clean up

	//finally
	return tb;
}

void runBinaryCustom(const unsigned char* tb, size_t size) {
	Toy_Interpreter interpreter;
	Toy_initInterpreter(&interpreter);

	//NOTE: suppress print output for testing
	Toy_setInterpreterPrint(&interpreter, noPrintFn);
	Toy_setInterpreterError(&interpreter, noErrorFn);

	Toy_runInterpreter(&interpreter, tb, size);
	Toy_freeInterpreter(&interpreter);
}

void runSourceCustom(const char* source) {
	size_t size = 0;
	const unsigned char* tb = compileStringCustom(source, &size);
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
	bool success = true;

	{
		//run each file in tests/scripts/
		char* filenames[] = {
			"access-parent-directory.toy",
			"arithmetic-without-operand.toy",
			"bad-function-identifier.toy",
			"declare-types-array.toy",
			"declare-types-dictionary-key.toy",
			"declare-types-dictionary-value.toy",
			"index-access-bugfix.toy",
			"index-arrays-non-integer.toy",
			"string-concat.toy",
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
				printf(TOY_CC_ERROR "Expected error did not occur in %s\n" TOY_CC_RESET, filenames[i]);
				success = false;
			}

			errorsTriggered = 0;
		}
	}

	if (!success) {
		return -1;
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

