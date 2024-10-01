#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//internal structure that holds the individual parts of a compiled routine
typedef struct Toy_Routine {
	unsigned char* param; //c-string params in sequence (could be moved below the jump table?)
	size_t paramCapacity;
	size_t paramCount;

	unsigned char* code; //the instruction set
	size_t codeCapacity;
	size_t codeCount;

	size_t* jumps; //each 'jump' is the starting address of an element within 'data'
	size_t jumpsCapacity;
	size_t jumpsCount;

	unsigned char* data; //{type,val} tuples of data
	size_t dataCapacity;
	size_t dataCount;

	unsigned char* subs; //subroutines, recursively
	size_t subsCapacity;
	size_t subsCount;
} Toy_Routine;

TOY_API void* Toy_compileRoutine(Toy_Ast* ast);