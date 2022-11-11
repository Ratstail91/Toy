#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

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

void error(char* msg) {
	printf(msg);
	exit(-1);
}

//utilities
typedef struct ArbitraryData {
	int value;
} ArbitraryData;

static int produce(Interpreter* interpreter, LiteralArray* arguments) {
	ArbitraryData* data = ALLOCATE(ArbitraryData, 1);
	data->value = 42;

	Literal o = TO_OPAQUE_LITERAL(data, 0);

	pushLiteralArray(&interpreter->stack, o);

	freeLiteral(o);

	return 1;
}

static int consume(Interpreter* interpreter, LiteralArray* arguments) {
	Literal o = popLiteralArray(arguments);

	Literal idn = o;

	if (parseIdentifierToValue(interpreter, &o)) {
		freeLiteral(idn);
	}

	if (IS_OPAQUE(o) && ((ArbitraryData*)(AS_OPAQUE(o)))->value == 42) {
		ArbitraryData* data = (ArbitraryData*)AS_OPAQUE(o);

		FREE(ArbitraryData, data);

		//all went well
		freeLiteral(o);
		return 0;
	}

	printf(ERROR "opaque failed: %d\n" RESET, IS_OPAQUE(o));

	exit(-1);
	return -1;
}

int main() {
	{
		size_t size = 0;
		char* source = readFile("scripts/opaque-data-type.toy", &size);
		unsigned char* tb = compileString(source, &size);
		free((void*)source);

		if (!tb) {
			return -1;
		}

		Interpreter interpreter;
		initInterpreter(&interpreter);

		injectNativeFn(&interpreter, "produce", produce);
		injectNativeFn(&interpreter, "consume", consume);

		//run teh script
		runInterpreter(&interpreter, tb, size);

		//clean up
		freeInterpreter(&interpreter);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

