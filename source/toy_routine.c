#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_memory.h"
#include "toy_opcodes.h"
#include "toy_value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void expand(void** handle, int* capacity, int* count, int amount) {
	if ((*count) + amount > (*capacity)) {
		int oldCapacity = (*capacity);

		while ((*count) + amount > (*capacity)) {
			(*capacity) = TOY_GROW_CAPACITY(*capacity);
		}
		(*handle) = TOY_GROW_ARRAY(unsigned char, (*handle), oldCapacity, (*capacity));
	}
}

static void emitByte(void** handle, int* capacity, int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

static void emitInt(void** handle, int* capacity, int* count, int bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

static void emitFloat(void** handle, int* capacity, int* count, float bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

//write instructions based on the AST types
#define EMIT_BYTE(rt, byte) \
	emitByte((void**)(&((*rt)->code)), &((*rt)->codeCapacity), &((*rt)->codeCount), byte);
#define EMIT_INT(rt, code, byte) \
	emitInt((void**)(&((*rt)->code)), &((*rt)->codeCapacity), &((*rt)->codeCount), byte);
#define EMIT_FLOAT(rt, code, byte) \
	emitFloat((void**)(&((*rt)->code)), &((*rt)->codeCapacity), &((*rt)->codeCount), byte);

static void writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast); //forward declare for recursion

static void writeInstructionValue(Toy_Routine** rt, Toy_AstValue ast) {
	//TODO: store more complex values in the data code
	EMIT_BYTE(rt, TOY_OPCODE_READ);
	EMIT_BYTE(rt, ast.value.type);

	//emit the raw value based on the type
	if (TOY_VALUE_IS_NULL(ast.value)) {
		//NOTHING - null's type data is enough

		//BUGFIX: 4-byte alignment
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);
	}
	else if (TOY_VALUE_IS_BOOLEAN(ast.value)) {
		EMIT_BYTE(rt, TOY_VALUE_AS_BOOLEAN(ast.value));

		//BUGFIX: 4-byte alignment
		EMIT_BYTE(rt, 0);
	}
	else if (TOY_VALUE_IS_INTEGER(ast.value)) {
		//BUGFIX: 4-byte alignment
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);

		EMIT_INT(rt, code, TOY_VALUE_AS_INTEGER(ast.value));
	}
	else if (TOY_VALUE_IS_FLOAT(ast.value)) {
		//BUGFIX: 4-byte alignment
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);

		EMIT_FLOAT(rt, code, TOY_VALUE_AS_FLOAT(ast.value));
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown value type\n" TOY_CC_RESET);
		exit(-1);
	}
}

static void writeInstructionUnary(Toy_Routine** rt, Toy_AstUnary ast) {
	//working with a stack means the child gets placed first
	writeRoutineCode(rt, ast.child);

	if (ast.flag == TOY_AST_FLAG_NEGATE) {
		EMIT_BYTE(rt, TOY_OPCODE_NEGATE);

		//BUGFIX: 4-byte alignment
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST unary flag found\n" TOY_CC_RESET);
		exit(-1);
	}
}

static void writeInstructionBinary(Toy_Routine** rt, Toy_AstBinary ast) {
	//left, then right, then the binary's operation
	writeRoutineCode(rt, ast.left);
	writeRoutineCode(rt, ast.right);

	if (ast.flag == TOY_AST_FLAG_ADD) {
		EMIT_BYTE(rt, TOY_OPCODE_ADD);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT) {
		EMIT_BYTE(rt, TOY_OPCODE_SUBTRACT);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY) {
		EMIT_BYTE(rt, TOY_OPCODE_MULTIPLY);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE) {
		EMIT_BYTE(rt, TOY_OPCODE_DIVIDE);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO) {
		EMIT_BYTE(rt, TOY_OPCODE_MODULO);
	}

	// else if (ast.flag == TOY_AST_FLAG_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_ADD_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_ADD);
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_SUBTRACT_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_SUBTRACT);
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_MULTIPLY_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_MULTIPLY);
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_DIVIDE_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_DIVIDE);
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_MODULO_ASSIGN) {
	// 	EMIT_BYTE(rt, TOY_OPCODE_MODULO);
	// 	EMIT_BYTE(rt, TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }

	else if (ast.flag == TOY_AST_FLAG_COMPARE_EQUAL) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_NOT) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_EQUAL);
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);

		EMIT_BYTE(rt, TOY_OPCODE_NEGATE); //TODO: squeeze these into one word
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);
		EMIT_BYTE(rt, 0);

		return;
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_LESS);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS_EQUAL) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_LESS_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_GREATER);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER_EQUAL) {
		EMIT_BYTE(rt, TOY_OPCODE_COMPARE_GREATER_EQUAL);
	}

	else if (ast.flag == TOY_AST_FLAG_AND) {
		EMIT_BYTE(rt, TOY_OPCODE_AND);
	}
	else if (ast.flag == TOY_AST_FLAG_OR) {
		EMIT_BYTE(rt, TOY_OPCODE_OR);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//BUGFIX: 4-byte alignment (covers most cases)
	EMIT_BYTE(rt, 0);
	EMIT_BYTE(rt, 0);
	EMIT_BYTE(rt, 0);
}

//routine structure
// static void writeRoutineParam(Toy_Routine* rt) {
// 	//
// }

static void writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast) {
	if (ast == NULL) {
		return;
	}

	//determine how to write each instruction based on the Ast
	switch(ast->type) {
		case TOY_AST_BLOCK:
			writeRoutineCode(rt, ast->block.child);
			writeRoutineCode(rt, ast->block.next);
			break;

		case TOY_AST_VALUE:
			writeInstructionValue(rt, ast->value);
			break;

		case TOY_AST_UNARY:
			writeInstructionUnary(rt, ast->unary);
			break;

		case TOY_AST_BINARY:
			writeInstructionBinary(rt, ast->binary);
			break;

		//other disallowed instructions
		case TOY_AST_GROUP:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Group shouldn't be used\n" TOY_CC_RESET);
			exit(-1);
			break;

		case TOY_AST_PASS:
			//NOTE: this should be disallowed, but for now it's required for testing
			// fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown pass\n" TOY_CC_RESET);
			// exit(-1);
			break;

		//meta instructions are disallowed
		case TOY_AST_ERROR:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown error\n" TOY_CC_RESET);
			exit(-1);
			break;

		case TOY_AST_END:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown end\n" TOY_CC_RESET);
			exit(-1);
			break;
	}
}

// static void writeRoutineJumps(Toy_Routine* rt) {
// 	//
// }

// static void writeRoutineData(Toy_Routine* rt) {
// 	//
// }

static void* writeRoutine(Toy_Routine* rt, Toy_Ast* ast) {
	//build the routine's parts
	//TODO: param
	//code
	writeRoutineCode(&rt, ast);
	EMIT_BYTE(&rt, TOY_OPCODE_RETURN); //temp terminator
	EMIT_BYTE(&rt, 0); //BUGFIX: 4-byte alignment
	EMIT_BYTE(&rt, 0);
	EMIT_BYTE(&rt, 0);
	//TODO: jumps
	//TODO: data

	//write the header and combine the parts
	void* buffer = TOY_ALLOCATE(unsigned char, 16);
	int capacity = 0, count = 0;
	// int paramAddr = 0, codeAddr = 0, jumpsAddr = 0, dataAddr = 0, subsAddr = 0;
	int codeAddr = 0;

	emitInt(&buffer, &capacity, &count, 0); //total size (overwritten later)
	emitInt(&buffer, &capacity, &count, rt->paramCount); //param count
	emitInt(&buffer, &capacity, &count, rt->jumpsCount); //jumps count
	emitInt(&buffer, &capacity, &count, rt->dataCount); //data count
	emitInt(&buffer, &capacity, &count, rt->subsCount); //routine count

	//generate blank spaces, cache their positions in the []Addr variables
	if (rt->paramCount > 0) {
		// paramAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //params
	}
	if (rt->codeCount > 0) {
		codeAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //code
	}
	if (rt->jumpsCount > 0) {
		// jumpsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //jumps
	}
	if (rt->dataCount > 0) {
		// dataAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //data
	}
	if (rt->subsCount > 0) {
		// subsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //subs
	}

	//append various parts to the buffer TODO: add the rest
	if (rt->codeCount > 0) {
		expand(&buffer, &capacity, &count, rt->codeCount);
		memcpy((buffer + count), rt->code, rt->codeCount);

		*((int*)(buffer + codeAddr)) = count;
		count += rt->codeCount;
	}

	//finally, record the total size within the header, and return the result
	*((int*)buffer) = count;

	return buffer;
}

//exposed functions
void* Toy_compileRoutine(Toy_Ast* ast) {
	//setup
	Toy_Routine rt;

	rt.param = NULL;
	rt.paramCapacity = 0;
	rt.paramCount = 0;

	rt.code = NULL;
	rt.codeCapacity = 0;
	rt.codeCount = 0;

	rt.jumps = NULL;
	rt.jumpsCapacity = 0;
	rt.jumpsCount = 0;

	rt.data = NULL;
	rt.dataCapacity = 0;
	rt.dataCount = 0;

	rt.subs = NULL;
	rt.subsCapacity = 0;
	rt.subsCount = 0;

	//build
	void * buffer = writeRoutine(&rt, ast);

	//cleanup the temp object
	TOY_FREE_ARRAY(unsigned char, rt.param, rt.paramCapacity);
	TOY_FREE_ARRAY(unsigned char, rt.code, rt.codeCapacity);
	TOY_FREE_ARRAY(int, rt.jumps, rt.jumpsCapacity);
	TOY_FREE_ARRAY(unsigned char, rt.data, rt.dataCapacity);
	TOY_FREE_ARRAY(unsigned char, rt.subs, rt.subsCapacity);

	return buffer;
}
