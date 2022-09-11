#pragma once

#include "common.h"
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

TOY_API void initCompiler(Compiler* compiler);
TOY_API void writeCompiler(Compiler* compiler, Node* node);
TOY_API void freeCompiler(Compiler* compiler);

//embed the header, data section, code section, function section, etc.
TOY_API unsigned char* collateCompiler(Compiler* compiler, int* size);
