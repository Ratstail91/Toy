#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../repl/lib_standard.h"
#include "../repl/lib_timer.h"

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

static int failedAsserts = 0;
static void assertWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, ERROR "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n" RESET); //default new line
}

static void errorWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, ERROR "%s" RESET, output);
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

void runBinaryWithLibrary(unsigned char* tb, size_t size, char* library, HookFn hook) {
	Interpreter interpreter;
	initInterpreter(&interpreter);

	//NOTE: supress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);
	setInterpreterAssert(&interpreter, assertWrapper);
	setInterpreterError(&interpreter, errorWrapper);

	//inject the standard libraries into this interpreter
	injectNativeHook(&interpreter, library, hook);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
}

typedef struct Payload {
	char* fname;
	char* libname;
	HookFn hook;
} Payload;

int main() {
	{
		//run each file in ../scripts/test/
		Payload payloads[] = {
			{"interactions.toy", "standard", hookStandard}, //interactions needs standard
			{"standard.toy", "standard", hookStandard},
			// {"timer.toy", "timer", hookTimer},
			{NULL, NULL, NULL}
		};

		for (int i = 0; payloads[i].fname; i++) {
			printf("Running %s\n", payloads[i].fname);

			char fname[128];
			snprintf(fname, 128, "../scripts/test/lib/%s", payloads[i].fname);

			//compile the source
			size_t size = 0;
			char* source = readFile(fname, &size);
			unsigned char* tb = compileString(source, &size);
			free((void*)source);

			if (!tb) {
				printf(ERROR "Failed to compile file: %s" RESET, fname);
			}

			runBinaryWithLibrary(tb, size, payloads[i].libname, payloads[i].hook);
		}
	}

	if (!failedAsserts) {
		printf(NOTICE "All good\n" RESET);
	}
	else {
		printf(WARN "Problems detected in libraries\n" RESET);
	}

	return failedAsserts;
}

