#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//internal structure that holds the individual parts of a compiled routine
typedef struct Toy_Routine {
	unsigned char* param; //c-string params in sequence (could be moved below the jump table?)
	int paramCapacity;
	int paramCount;

	unsigned char* code; //the instruction set
	int codeCapacity;
	int codeCount;

	int* jumps; //each 'jump' is the starting address of an element within 'data'
	int jumpsCapacity;
	int jumpsCount;

	unsigned char* data; //{type,val} tuples of data
	int dataCapacity;
	int dataCount;

	unsigned char* subs; //subroutines, recursively
	int subsCapacity;
	int subsCount;
} Toy_Routine;

TOY_API void* Toy_compileRoutine(Toy_Ast* ast);