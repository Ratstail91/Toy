#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//routine - holds the individual parts of a compiled routine
typedef struct Toy_Routine {
	unsigned char* param; //c-string params in sequence
	int paramCapacity;
	int paramCount;

	unsigned char* code; //the instruction set
	int codeCapacity;
	int codeCount;

	unsigned char* data; //{type,val} tuples of data
	int dataCapacity;
	int dataCount;

	int* jump; //each 'jump' is the starting address of an element within 'data'
	int jumpCapacity;
	int jumpCount;

	//TODO: duplicate the data and jumps for subroutines
} Toy_Routine;

TOY_API Toy_Routine Toy_compileRoutine(Toy_Ast* ast);
TOY_API void Toy_freeRoutine(Toy_Routine routine);
