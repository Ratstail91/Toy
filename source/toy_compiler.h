#pragma once

#include "toy_common.h"
#include "toy_opcodes.h"
#include "toy_ast_node.h"
#include "toy_literal_array.h"

//the compiler takes the nodes, and turns them into sequential chunks of bytecode, saving literals to an external array
typedef struct Toy_Compiler {
	Toy_LiteralArray literalCache;
	unsigned char* bytecode;
	int capacity;
	int count;
	bool panic;
} Toy_Compiler;

TOY_API void Toy_initCompiler(Toy_Compiler* compiler);
TOY_API void Toy_writeCompiler(Toy_Compiler* compiler, Toy_ASTNode* node);
TOY_API void Toy_freeCompiler(Toy_Compiler* compiler);

//embed the header, data section, code section, function section, etc.
TOY_API unsigned char* Toy_collateCompiler(Toy_Compiler* compiler, int* size);
