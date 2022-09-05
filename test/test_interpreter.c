#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
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
	Node* node = scanParser(&parser);
	while(node != NULL) {
		//pack up and leave
		if (node->type == NODE_ERROR) {
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

	//NOTE: supress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);

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

		Node* node = scanParser(&parser);

		//write
		writeCompiler(&compiler, node);

		//collate
		int size = 0;
		unsigned char* bytecode = collateCompiler(&compiler, &size);

		//NOTE: supress print output for testing
		setInterpreterPrint(&interpreter, noPrintFn);

		//run
		runInterpreter(&interpreter, bytecode, size);

		//cleanup
		freeNode(node);
		freeParser(&parser);
		freeCompiler(&compiler);
		freeInterpreter(&interpreter);
	}

	{
		//run each file in ../scripts/test/
		int count = 15;
		char* filenames[] = {
			"arithmetic.toy",
			"casting.toy",
			"comparisons.toy",
			"dot-and-matrix.toy",
			"functions.toy",
			"imports-and-exports.toy",
			"jumps.toy",
			"jumps-in-functions.toy",
			"logicals.toy",
			"long-array.toy",
			"long-dictionary.toy",
			"long-literals.toy",
			"native-functions.toy",
			"panic-within-functions.toy", 
			"types.toy"
		};

		for (int i = 0; i < count; i++) {
			printf("Running %s\n", filenames[i]);

			char buffer[128];
			snprintf(buffer, 128, "../scripts/test/%s", filenames[i]);

			runSourceFile(buffer);
		}
	}

	{
		//read source
		size_t dummy;
		size_t exportSize, importSize;
		char* exportSource = readFile("../scripts/test/separate-exports.toy", &dummy);
		char* importSource = readFile("../scripts/test/separate-imports.toy", &dummy);

		//compile
		unsigned char* exportBinary = compileString(exportSource, &exportSize);
		unsigned char* importBinary = compileString(importSource, &importSize);

		//run the interpreter over both binaries
		Interpreter interpreter;
		initInterpreter(&interpreter);

		//NOTE: supress print output for testing
		setInterpreterPrint(&interpreter, noPrintFn);

		runInterpreter(&interpreter, exportBinary, exportSize); //automatically frees the binary data

		resetInterpreter(&interpreter);

		runInterpreter(&interpreter, importBinary, importSize); //automatically frees the binary data

		freeInterpreter(&interpreter);

		//cleanup
		free((void*)exportSource);
		free((void*)importSource);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

