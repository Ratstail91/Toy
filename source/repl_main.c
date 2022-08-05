#include "debug.h"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
//#include "toy.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//read a file and return it as a char array
char* readFile(char* path) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize + 1);

	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\"\n", path);
		exit(74);
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\"\n", path);
		exit(74);
	}

	fclose(file);

	buffer[bytesRead] = '\0';

	return buffer;
}

/*
//run functions
void runString(char* source) {
	Lexer lexer;
	Parser parser;
	Toy toy;

	initLexer(&lexer, source);
	initParser(&parser, &lexer);
	initToy(&toy);

	Chunk* chunk = scanParser(&parser);

	if (chunk->count > 1 && command.verbose) {
		printChunk(chunk, "    ");
	}

	executeChunk(&toy, chunk);

	freeChunk(chunk);

	freeToy(&toy);
	freeParser(&parser);
}

void runFile(char* fname) {
	char* source = readFile(fname);

	runString(source);

	free((void*)source);
}

void repl() {
	const int size = 2048;
	char input[size];
	memset(input, 0, size);

	Parser parser;
	Toy toy;

	initToy(&toy);

	for(;;) {
		printf(">");
		fgets(input, size, stdin);

		//setup
		Lexer lexer;

		initLexer(&lexer, input);
		initParser(&parser, &lexer);

		//run
		Chunk* chunk = scanParser(&parser);

		if (chunk->count > 1 && command.verbose) {
			printChunk(chunk, "    ");
		}

		//clean up the memory
		if (parser.error) {
			freeChunk(chunk);
			freeParser(&parser);
			continue;
		}

		executeChunk(&toy, chunk);

		if (toy.panic) {
			toy.panic = false;
			freeChunk(chunk);
			freeParser(&parser);
			continue;
		}

		freeChunk(chunk);

		//cleanup
		freeParser(&parser);
	}

	freeToy(&toy);
}
*/

void debug() {
	Lexer lexer;
	Parser parser;
	Compiler compiler;

	char* source = readFile(command.filename);

	initLexer(&lexer, source);
	initParser(&parser, &lexer);
	initCompiler(&compiler);

	//run the parser until the end of the source
	Node* node = scanParser(&parser);
	while(node != NULL) {
		writeCompiler(&compiler, node);

		freeNode(node);

		node = scanParser(&parser);
	}

	//get the data dump
	int size = 0;
	const char* tb = collateCompiler(&compiler, &size);

	dissectBytecode(tb, size);

	//cleanup
	FREE_ARRAY(char, tb, size);
	freeCompiler(&compiler);
	freeParser(&parser);
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

	if (command.filename) {
		debug();
//		runFile(command.filename);
		return 0;
	}

	if (command.source) {
//		runString(command.source);

		// Lexer lexer;
		// initLexer(&lexer, command.source);

		// //debugging
		// while(true) {
		// 	Token token = scanLexer(&lexer);

		// 	if (token.type == TOKEN_EOF) {
		// 		break;
		// 	}
		// }

		return 0;
	}

//	repl();

	return 0;
}