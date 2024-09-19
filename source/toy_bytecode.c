#include "toy_bytecode.h"

#include "toy_memory.h"

#include <stdio.h>
#include <string.h>

//utils
static void expand(Toy_Bytecode* bc, int amount) {
	if (bc->count + amount > bc->capacity) {
		int oldCapacity = bc->capacity;

		bc->capacity = TOY_GROW_CAPACITY(oldCapacity);
		bc->ptr = TOY_GROW_ARRAY(unsigned char, bc->ptr, oldCapacity, bc->capacity);
	}
}

static void emitByte(Toy_Bytecode* bc, unsigned char byte) {
	expand(bc, 1);
	bc->ptr[bc->count++] = byte;
}

static void writeModule(Toy_Bytecode* bc, Toy_Ast* ast) {
	//
}

//bytecode
static void writeBytecodeHeader(Toy_Bytecode* bc) {
	emitByte(bc, TOY_VERSION_MAJOR);
	emitByte(bc, TOY_VERSION_MINOR);
	emitByte(bc, TOY_VERSION_PATCH);

	//check strlen for the build string
	const char* build = Toy_private_version_build();
	int len = (int)strlen(build) + 1;

	expand(bc, len);
	sprintf((char*)(bc->ptr + bc->count), "%.*s", len, build);
	bc->count += len;
}

static void writeBytecodeBody(Toy_Bytecode* bc, Toy_Ast* ast) {
	//a 'module' is a routine that runs at the root-level of a file
	//since routines can be recursive, this distinction is important
	//eventually, the bytecode may support multiple modules packed into one file
	writeModule(bc, ast);
}

//exposed functions
Toy_Bytecode Toy_compileBytecode(Toy_Ast* ast) {
	//setup
	Toy_Bytecode bc;

	bc.ptr = NULL;
	bc.capacity = 0;
	bc.count = 0;

	//build
	writeBytecodeHeader(&bc);
	writeBytecodeBody(&bc, ast);

	return bc;
}

void Toy_freeBytecode(Toy_Bytecode bc) {
	TOY_FREE_ARRAY(unsigned char, bc.ptr, bc.capacity);
}
