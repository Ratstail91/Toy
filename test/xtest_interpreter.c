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
		if (node->type == AST_NODEERROR) {
			printf(ERROR "error node detected\n" RESET);
			freeNode(node);
			freeCompiler(&compiler);
			freeParser(&parser);
			return NULL;
		}

		writeCompiler(&compiler, node);
		freeNode(node);
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
	setInterpreterAssert(&interpreter, noAssertFn);

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
		freeNode(node);
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
			"imports-and-exports.toy",
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
			"types.toy",
			NULL
		};

		for (int i = 0; filenames[i]; i++) {
			printf("Running %s\n", filenames[i]);

			char buffer[128];
			snprintf(buffer, 128, "scripts/%s", filenames[i]);

			runSourceFile(buffer);
		}
	}

	{
		//read source
		size_t dummy;
		size_t exportSize, importSize;
		char* exportSource = readFile("scripts/separate-exports.toy", &dummy);
		char* importSource = readFile("scripts/separate-imports.toy", &dummy);

		//compile
		unsigned char* exportBinary = compileString(exportSource, &exportSize);
		unsigned char* importBinary = compileString(importSource, &importSize);

		//run the interpreter over both binaries
		Interpreter interpreter;
		initInterpreter(&interpreter);

		//NOTE: supress print output for testing
		setInterpreterPrint(&interpreter, noPrintFn);
		setInterpreterAssert(&interpreter, noAssertFn);

		runInterpreter(&interpreter, exportBinary, exportSize); //automatically frees the binary data

		resetInterpreter(&interpreter);

		runInterpreter(&interpreter, importBinary, importSize); //automatically frees the binary data

		freeInterpreter(&interpreter);

		//cleanup
		free((void*)exportSource);
		free((void*)importSource);
	}

	//1, to allow for the assertion test
	if (ignoredAssertions > 1) {
		fprintf(stderr, ERROR "Assertions hidden: %d\n", ignoredAssertions);
		return -1;
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

