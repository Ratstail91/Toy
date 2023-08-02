#include "repl_tools.h"
#include "lib_toy_version_info.h"
#include "lib_standard.h"
#include "lib_random.h"
#include "lib_runner.h"
#include "lib_math.h"

#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include <stdio.h>
#include <stdlib.h>

//IO functions
const unsigned char* Toy_readFile(const char* path, size_t* fileSize) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, TOY_CC_ERROR "Could not open file \"%s\"\n" TOY_CC_RESET, path);
		return NULL;
	}

	fseek(file, 0L, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);

	unsigned char* buffer = (unsigned char*)malloc(*fileSize + 1);

	if (buffer == NULL) {
		fprintf(stderr, TOY_CC_ERROR "Not enough memory to read \"%s\"\n" TOY_CC_RESET, path);
		return NULL;
	}

	size_t bytesRead = fread(buffer, sizeof(unsigned char), *fileSize, file);

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

	//step 1 - run the parser until the end of the source
	Toy_ASTNode* node = Toy_scanParser(&parser);
	while(node != NULL) {
		//on error, pack up and leave
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

	//step 2 - get the bytecode dump
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
	Toy_injectNativeHook(&interpreter, "toy_version_info", Toy_hookToyVersionInfo);
	Toy_injectNativeHook(&interpreter, "standard", Toy_hookStandard);
	Toy_injectNativeHook(&interpreter, "random", Toy_hookRandom);
	Toy_injectNativeHook(&interpreter, "runner", Toy_hookRunner);
	Toy_injectNativeHook(&interpreter, "math", Toy_hookMath);

	Toy_runInterpreter(&interpreter, tb, (int)size);
	Toy_freeInterpreter(&interpreter);
}

void Toy_runBinaryFile(const char* fname) {
	size_t size = 0; //not used
	const unsigned char* tb = Toy_readFile(fname, &size);
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
	const char* source = (const char*)Toy_readFile(fname, &size);
	if (!source) {
		return;
	}
	Toy_runSource(source);
	free((void*)source);
}

//utils for debugging the header
static unsigned char readByte(const unsigned char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	*count += 1;
	return ret;
}

static const char* readString(const unsigned char* tb, int* count) {
	const unsigned char* ret = tb + *count;
	*count += (int)strlen((char*)ret) + 1; //+1 for null character
	return (const char*)ret;
}

void Toy_parseBinaryFileHeader(const char* fname) {
	size_t size = 0; //not used
	const unsigned char* tb = Toy_readFile(fname, &size);
	if (!tb || size < 4) {
		return;
	}

	int count = 0;

	//header section
	const unsigned char major = readByte(tb, &count);
	const unsigned char minor = readByte(tb, &count);
	const unsigned char patch = readByte(tb, &count);

	const char* build = readString(tb, &count);

	printf("Toy Programming Language Interpreter Version %d.%d.%d (interpreter built on %s)\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);

	printf("Toy Programming Language Bytecode Version ");

	//print the output
	if (major == TOY_VERSION_MAJOR && minor == TOY_VERSION_MINOR && patch == TOY_VERSION_PATCH) {
		printf("%d.%d.%d", major, minor, patch);
	}
	else {
		printf(TOY_CC_FONT_YELLOW TOY_CC_BACK_BLACK "%d.%d.%d" TOY_CC_RESET, major, minor, patch);
	}

	printf(" (interpreter built on ");

	if (strncmp(build, TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD)) == 0) {
		printf("%s", build);
	}
	else {
		printf(TOY_CC_FONT_YELLOW TOY_CC_BACK_BLACK "%s" TOY_CC_RESET, build);
	}

	printf(")\n");

	//cleanup
	free((void*)tb);
}