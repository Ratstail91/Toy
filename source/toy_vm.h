#pragma once

#include "toy_common.h"

#include "toy_stack.h"

typedef struct Toy_VM {
	//bytecode - raw instructions that are being executed
	unsigned char* program;
	int programSize;

	int paramCount;
	int jumpsCount;
	int dataCount;
	int subsCount;

	int paramAddr;
	int codeAddr;
	int jumpsAddr;
	int dataAddr;
	int subsAddr;

	int programCounter;

	//scope - block-level key/value pairs

	//stack - immediate-level values only
	Toy_Stack stack;
} Toy_VM;

TOY_API void Toy_initVM(Toy_VM* vm);
TOY_API void Toy_bindVM(Toy_VM* vm, unsigned char* program);
TOY_API void Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);

//TODO: inject extra data
