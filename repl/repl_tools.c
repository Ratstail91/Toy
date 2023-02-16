#include "repl_tools.h"
#include "lib_about.h"
#include "lib_standard.h"
#include "lib_runner.h"

#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include <stdio.h>
#include <stdlib.h>

//IO functions
const char* Toy_readFile(const char* path, size_t* fileSize) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, TOY_CC_ERROR "Could not open file \"%s\"\n" TOY_CC_RESET, path);
		return NULL;
	}

	fseek(file, 0L, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(*fileSize + 1);

	if (buffer == NULL) {
		fprintf(stderr, TOY_CC_ERROR "Not enough memory to read \"%s\"\n" TOY_CC_RESET, path);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(char), *fileSize, file);

	buffer[*fileSize] = '\0'; //NOTE: fread doesn't append this

	if (bytesRead < *fileSize) {
		fprintf(stderr, TOY_CC_ERROR "Could not read file \"%s\"\n" TOY_CC_RESET, path);
		return NULL;
	}

	fclose(file);

	return buffer;
}

int Toy_writeFile(const char* path, const unsigned char* bytes, size_t size) {
	FILE* file = fopen(path, "wb");

	if (file == NULL) {
		fprintf(stderr, TOY_CC_ERROR "Could not open file \"%s\"\n" TOY_CC_RESET, path);
		return -1;
	}

	size_t written = fwrite(bytes, size, 1, file);

	if (written != 1) {
		fprintf(stderr, TOY_CC_ERROR "Could not write file \"%s\"\n" TOY_CC_RESET, path);
		return -1;
	}

	fclose(file);

	return 0;
}

//repl functions
const unsigned char* Toy_compileString(const char* source, size_t* size) {
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

void Toy_runBinary(const unsigned char* tb, size_t size) {
	Toy_Interpreter interpreter;
	Toy_initInterpreter(&interpreter);

	//inject the libs
	Toy_injectNativeHook(&interpreter, "about", Toy_hookAbout);
	Toy_injectNativeHook(&interpreter, "standard", Toy_hookStandard);
	Toy_injectNativeHook(&interpreter, "runner", Toy_hookRunner);

	Toy_runInterpreter(&interpreter, tb, (int)size);
	Toy_freeInterpreter(&interpreter);
}

void Toy_runBinaryFile(const char* fname) {
	size_t size = 0; //not used
	const unsigned char* tb = (const unsigned char*)Toy_readFile(fname, &size);
	if (!tb) {
		return;
	}
	Toy_runBinary(tb, size);
	//interpreter takes ownership of the binary data
}

void Toy_runSource(const char* source) {
	size_t size = 0;
	const unsigned char* tb = Toy_compileString(source, &size);
	if (!tb) {
		return;
	}

	Toy_runBinary(tb, size);
}

void Toy_runSourceFile(const char* fname) {
	size_t size = 0; //not used
	const char* source = Toy_readFile(fname, &size);
	if (!source) {
		return;
	}
	Toy_runSource(source);
	free((void*)source);
}
