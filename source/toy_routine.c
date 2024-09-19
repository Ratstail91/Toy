#include "toy_routine.h"

#include "toy_memory.h"

#include <stdio.h>
#include <string.h>

//utils
static void expand(void** handle, int* capacity, int* count, int amount) {
	while ((*count) + amount > (*capacity)) {
		int oldCapacity = (*capacity);

		(*capacity) = TOY_GROW_CAPACITY(oldCapacity);
		(*handle) = TOY_GROW_ARRAY(unsigned char, (*handle), oldCapacity, (*capacity));
	}
}

static void emitByte(void** handle, int* capacity, int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

//routine
//TODO

//exposed functions
Toy_Routine Toy_compileRoutine(Toy_Ast* ast) {
	//setup
	Toy_Routine rt;

	rt.param = NULL;
	rt.paramCapacity = 0;
	rt.paramCount = 0;

	rt.code = NULL;
	rt.codeCapacity = 0;
	rt.codeCount = 0;

	rt.data = NULL;
	rt.dataCapacity = 0;
	rt.dataCount = 0;

	rt.jump = NULL;
	rt.jumpCapacity = 0;
	rt.jumpCount = 0;

	//build
	//TODO

	return rt;
}

void freeRoutine(Toy_Routine rt) {
	TOY_FREE_ARRAY(unsigned char, rt.param, rt.paramCapacity);
	TOY_FREE_ARRAY(unsigned char, rt.code, rt.codeCapacity);
	TOY_FREE_ARRAY(unsigned char, rt.data, rt.dataCapacity);
	TOY_FREE_ARRAY(int, rt.jump, rt.jumpCapacity);
}