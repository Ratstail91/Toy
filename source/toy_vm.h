#pragma once

#include "toy_common.h"

#include "toy_stack.h"
#include "toy_bucket.h"

typedef struct Toy_VM {
	//hold the raw bytecode
	unsigned char* bc;

	//raw instructions to be executed
	unsigned char* routine;
	unsigned int routineSize;

	unsigned int paramSize;
	unsigned int jumpsSize;
	unsigned int dataSize;
	unsigned int subsSize;

	unsigned int paramAddr;
	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int dataAddr;
	unsigned int subsAddr;

	unsigned int routineCounter;

	//heap - block-level key/value pairs
	//TODO: needs string util for identifiers

	//stack - immediate-level values only
	Toy_Stack* stack;

	//easy access to memory
	Toy_Bucket* stringBucket;
} Toy_VM;

TOY_API void Toy_bindVM(Toy_VM* vm, unsigned char* bytecode); //process the version data
TOY_API void Toy_bindVMToRoutine(Toy_VM* vm, unsigned char* routine); //process the routine only

TOY_API void Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);
TOY_API void Toy_resetVM(Toy_VM* vm);

//TODO: inject extra data
