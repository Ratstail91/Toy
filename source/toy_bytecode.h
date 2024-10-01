#pragma once

#include "toy_common.h"
#include "toy_ast.h"

typedef struct Toy_Bytecode {
	unsigned char* ptr;
	size_t capacity;
	size_t count;
} Toy_Bytecode;

TOY_API Toy_Bytecode Toy_compileBytecode(Toy_Ast* ast);
TOY_API void Toy_freeBytecode(Toy_Bytecode bc);
