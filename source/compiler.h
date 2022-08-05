#pragma once

#include "opcodes.h"

#include "node.h"
#include "literal_array.h"

//the compiler takes the nodes, and turns them into sequential chunks of bytecode, saving literals to an external array
typedef struct Compiler {
	LiteralArray literalCache;
	unsigned char* bytecode;
	int capacity;
	int count;
} Compiler;

void initCompiler(Compiler* compiler);
void writeCompiler(Compiler* compiler, Node* node);
void freeCompiler(Compiler* compiler);

//embed the header with version information, data section, code section, etc.
char* collateCompiler(Compiler* compiler, int* size);
