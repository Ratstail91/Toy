#include "debug.h"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//IO functions
char* readFile(char* path, size_t* fileSize) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		exit(-1);
	}

	fseek(file, 0L, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(*fileSize);

	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\"\n", path);
		exit(-1);
	}

	size_t bytesRead = fread(buffer, sizeof(char), *fileSize, file);

	if (bytesRead < *fileSize) {
		fprintf(stderr, "Could not read file \"%s\"\n", path);
		exit(-1);
	}

	fclose(file);

	return buffer;
}

void writeFile(char* path, unsigned char* bytes, size_t size) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		exit(-1);
	}

	int written = fwrite(bytes, size, 1, file);

	if (written != 1) {
		fprintf(stderr, "Could not write file \"%s\"\n", path);
		exit(-1);
	}

	fclose(file);
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
	initInterpreter(&interpreter, tb, size);
	runInterpreter(&interpreter);
	freeInterpreter(&interpreter);
}

void runBinaryFile(char* fname) {
	size_t size = 0; //not used
	unsigned char* tb = (unsigned char*)readFile(fname, &size);
	runBinary(tb, size);
	//interpreter takes ownership of the binary data
}

void runSource(char* source) {
	size_t size;
	unsigned char* tb = compileString(source, &size);
	runBinary(tb, size);
}

void runSourceFile(char* fname) {
	size_t size = 0; //not used
	char* source = readFile(fname, &size);
	runSource(source);
	free((void*)source);
}

void repl() {
	//repl does it's own thing for now
	bool error = false;

	const int size = 2048;
	char input[size];
	memset(input, 0, size);

	Interpreter interpreter; //persist the interpreter for the scopes

	for(;;) {
		printf("> ");
		fgets(input, size, stdin);

		//setup this iteration
		Lexer lexer;
		Parser parser;
		Compiler compiler;

		initLexer(&lexer, input);
		initParser(&parser, &lexer);
		initCompiler(&compiler);

		//run this iteration
		Node* node = scanParser(&parser);
		while(node != NULL) {
			//pack up and restart
			if (node->type == NODE_ERROR) {
				error = true;
				freeNode(node);
				break;
			}

			writeCompiler(&compiler, node);
			freeNode(node);
			node = scanParser(&parser);
		}

		if (!error) {
			//get the bytecode dump
			int size = 0;
			unsigned char* tb = collateCompiler(&compiler, &size);

			// for (int i = 0; i < size; i++) {
			// 	printf("%d ", tb[i]);
			// }

			// printf("\n");

			//run the bytecode
			initInterpreter(&interpreter, tb, size);
			runInterpreter(&interpreter);
			freeInterpreter(&interpreter); //TODO: option to retain the scopes
		}

		//clean up this iteration
		freeCompiler(&compiler);
		freeParser(&parser);
		error = false;
	}

	freeInterpreter(&interpreter);
}

void debug() {
	LiteralDictionary dictionary;

	initLiteralDictionary(&dictionary);

	setLiteralDictionary(&dictionary, TO_IDENTIFIER_LITERAL("variable", MASK_INTEGER), TO_INTEGER_LITERAL(2));
	printLiteral( getLiteralDictionary(&dictionary, TO_IDENTIFIER_LITERAL("variable", MASK_INTEGER)) );

	freeLiteralDictionary(&dictionary);
}

//entry point
int main(int argc, const char* argv[]) {
	initCommand(argc, argv);

	//command specific actions
	if (command.error) {
		usageCommand(argc, argv);
		return 0;
	}

	if (command.help) {
		helpCommand(argc, argv);
		return 0;
	}

	if (command.version) {
		copyrightCommand(argc, argv);
		return 0;
	}

	//print this until the interpreter meets the specification
	if (command.verbose) {
		printf("Warning! This interpreter is a work in progress, it does not yet meet the %d.%d.%d specification.\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH);
	}

	//run binary
	if (command.binaryfile) {
		runBinaryFile(command.binaryfile);
		return 0;
	}

	//run source file
	if (command.sourcefile) {
		runSourceFile(command.sourcefile);
		return 0;
	}

	//compile source file
	if (command.compilefile) {
		size_t size = 0;
		char* source = readFile(command.compilefile, &size);
		unsigned char* tb = compileString(source, &size);
		writeFile(command.outfile, tb, size);
		return 0;
	}

	//run from stdin
	if (command.source) {
		runSource(command.source);
		return 0;
	}

	// debug();
	repl();

	return 0;
}