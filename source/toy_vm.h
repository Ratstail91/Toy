#pragma once

#include "toy_common.h"

#include "toy_stack.h"

typedef struct Toy_VM {
	//hold the raw bytecode
	unsigned char* bc;
	int bcSize;

	//raw instructions to be executed
	unsigned char* routine;
	int routineSize;

	int paramCount;
	int jumpsCount;
	int dataCount;
	int subsCount;

	int paramAddr;
	int codeAddr;
	int jumpsAddr;
	int dataAddr;
	int subsAddr;

	int routineCounter;

	//heap - block-level key/value pairs
	//TODO: needs string util for identifiers

	//stack - immediate-level values only
	Toy_Stack* stack;
} Toy_VM;

TOY_API void Toy_bindVM(Toy_VM* vm, unsigned char* bytecode, int bytecodeSize); //process the version data
TOY_API void Toy_bindVMToRoutine(Toy_VM* vm, unsigned char* routine); //process the routine only

TOY_API void Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);
TOY_API void Toy_resetVM(Toy_VM* vm);

//TODO: inject extra data
