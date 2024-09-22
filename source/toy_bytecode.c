#include "toy_bytecode.h"
#include "toy_console_colors.h"

#include "toy_memory.h"
#include "toy_routine.h"

#include <stdio.h>
#include <string.h>

//utils
static void expand(Toy_Bytecode* bc, int amount) {
	if (bc->count + amount > bc->capacity) {
		int oldCapacity = bc->capacity;

		while (bc->count + amount > bc->capacity) {
			bc->capacity = TOY_GROW_CAPACITY(bc->capacity);
		}
		bc->ptr = TOY_GROW_ARRAY(unsigned char, bc->ptr, oldCapacity, bc->capacity);
	}
}

static void emitByte(Toy_Bytecode* bc, unsigned char byte) {
	expand(bc, 1);
	bc->ptr[bc->count++] = byte;
}

//bytecode
static void writeBytecodeHeader(Toy_Bytecode* bc) {
	emitByte(bc, TOY_VERSION_MAJOR);
	emitByte(bc, TOY_VERSION_MINOR);
	emitByte(bc, TOY_VERSION_PATCH);

	//check strlen for the build string
	const char* build = Toy_private_version_build();
	int len = (int)strlen(build) + 1;

	//BUGFIX: ensure the end of the header has 4-byte alignment
	if (len % 4 != 1) { //1 to fill the 4th byte above
		len += 4 - (len % 4) +1; //ceil
	}

	expand(bc, len);
	memcpy(bc->ptr + bc->count, build, len);
	bc->count += len;
}

static void writeBytecodeBody(Toy_Bytecode* bc, Toy_Ast* ast) {
	//a 'module' is a routine that runs at the root-level of a file
	//since routines can be recursive, this distinction is important
	//eventually, the bytecode may support multiple modules packed into one file
	void* module = Toy_compileRoutine(ast);

	int len = ((int*)module)[0];

	expand(bc, len);
	memcpy(bc->ptr + bc->count, module, len);
	bc->count += len;
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
