#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void expand(void** handle, unsigned int* capacity, unsigned int* count, unsigned int amount) {
	if ((*count) + amount > (*capacity)) {
		while ((*count) + amount > (*capacity)) {
			(*capacity) = (*capacity) < 8 ? 8 : (*capacity) * 2;
		}
		(*handle) = realloc((*handle), (*capacity));

		if ((*handle) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate %d space for a part of 'Toy_Routine'\n" TOY_CC_RESET, (int)(*capacity));
			exit(1);
		}
	}
}

static void emitByte(void** handle, unsigned int* capacity, unsigned int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

static void emitInt(void** handle, unsigned int* capacity, unsigned int* count, unsigned int bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

static void emitFloat(void** handle, unsigned int* capacity, unsigned int* count, float bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

//write instructions based on the AST types
#define EMIT_BYTE(rt, part, byte) \
	emitByte((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), byte);
#define EMIT_INT(rt, part, bytes) \
	emitInt((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes);
#define EMIT_FLOAT(rt, part, bytes) \
	emitFloat((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes);

static void emitToJumpTable(Toy_Routine** rt, unsigned int startAddr) {
	EMIT_INT(rt, code, (*rt)->jumpsCount); //mark the jump index in the code
	EMIT_INT(rt, jumps, startAddr); //save address at the jump index
}

static void emitString(Toy_Routine** rt, Toy_String* str) {
	//4-byte alignment
	unsigned int length = str->length + 1;
	if (length % 4 != 0) {
		length += 4 - (length % 4); //ceil
	}

	//grab the current start address
	unsigned int startAddr = (*rt)->dataCount;

	//move the string into the data section
	expand((void**)(&((*rt)->data)), &((*rt)->dataCapacity), &((*rt)->dataCount), (*rt)->dataCount + length);

	if (str->type == TOY_STRING_NODE) {
		char* buffer = Toy_getStringRawBuffer(str);
		memcpy((*rt)->data + (*rt)->dataCount, buffer, str->length + 1);
		free(buffer);
	}
	else if (str->type == TOY_STRING_LEAF) {
		memcpy((*rt)->data + (*rt)->dataCount, str->as.leaf.data, str->length + 1);
	}
	else if (str->type == TOY_STRING_NAME) {
		memcpy((*rt)->data + (*rt)->dataCount, str->as.name.data, str->length + 1);
	}

	(*rt)->dataCount += length;

	//mark the jump position
	emitToJumpTable(rt, startAddr);
}

static void writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast); //forward declare for recursion

static void writeInstructionValue(Toy_Routine** rt, Toy_AstValue ast) {
	EMIT_BYTE(rt, code, TOY_OPCODE_READ);
	EMIT_BYTE(rt, code, ast.value.type);

	//emit the raw value based on the type
	if (TOY_VALUE_IS_NULL(ast.value)) {
		//NOTHING - null's type data is enough

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
	}
	else if (TOY_VALUE_IS_BOOLEAN(ast.value)) {
		EMIT_BYTE(rt, code, TOY_VALUE_AS_BOOLEAN(ast.value));

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
	}
	else if (TOY_VALUE_IS_INTEGER(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_INT(rt, code, TOY_VALUE_AS_INTEGER(ast.value));
	}
	else if (TOY_VALUE_IS_FLOAT(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_FLOAT(rt, code, TOY_VALUE_AS_FLOAT(ast.value));
	}
	else if (TOY_VALUE_IS_STRING(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		emitString(rt, TOY_VALUE_AS_STRING(ast.value));
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
		EMIT_BYTE(rt, code, TOY_OPCODE_NEGATE);

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
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
		EMIT_BYTE(rt, code,TOY_OPCODE_ADD);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT) {
		EMIT_BYTE(rt, code,TOY_OPCODE_SUBTRACT);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY) {
		EMIT_BYTE(rt, code,TOY_OPCODE_MULTIPLY);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DIVIDE);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO) {
		EMIT_BYTE(rt, code,TOY_OPCODE_MODULO);
	}

	// else if (ast.flag == TOY_AST_FLAG_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_ADD_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ADD);
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_SUBTRACT_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_SUBTRACT);
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_MULTIPLY_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_MULTIPLY);
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_DIVIDE_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_DIVIDE);
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }
	// else if (ast.flag == TOY_AST_FLAG_MODULO_ASSIGN) {
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_MODULO);
	// 	EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN);
	// 	//TODO: emit the env symbol to store TOP(S) within
	// }

	else if (ast.flag == TOY_AST_FLAG_COMPARE_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_NOT) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_EQUAL);
		EMIT_BYTE(rt, code,TOY_OPCODE_NEGATE); //squeezed into one word
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		return;
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_LESS);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_LESS_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_GREATER);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_GREATER_EQUAL);
	}

	else if (ast.flag == TOY_AST_FLAG_AND) {
		EMIT_BYTE(rt, code,TOY_OPCODE_AND);
	}
	else if (ast.flag == TOY_AST_FLAG_OR) {
		EMIT_BYTE(rt, code,TOY_OPCODE_OR);
	}
	else if (ast.flag == TOY_AST_FLAG_CONCAT) {
		EMIT_BYTE(rt, code, TOY_OPCODE_CONCAT);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment (covers most cases)
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
}

static void writeInstructionGroup(Toy_Routine** rt, Toy_AstGroup ast) {
	writeRoutineCode(rt, ast.child);
}

static void writeInstructionPrint(Toy_Routine** rt, Toy_AstPrint ast) {
	//the thing to print
	writeRoutineCode(rt, ast.child);

	//output the print opcode
	EMIT_BYTE(rt, code,TOY_OPCODE_PRINT);

	//4-byte alignment
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
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

		case TOY_AST_GROUP:
			writeInstructionGroup(rt, ast->group);
			break;

		case TOY_AST_PRINT:
			writeInstructionPrint(rt, ast->print);
			break;

		//meta instructions are disallowed
		case TOY_AST_PASS:
			//NOTE: this should be disallowed, but for now it's required for testing
			// fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown pass\n" TOY_CC_RESET);
			// exit(-1);
			break;

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

static void* writeRoutine(Toy_Routine* rt, Toy_Ast* ast) {
	//build the routine's parts
	//TODO: param
	//code
	writeRoutineCode(&rt, ast);
	EMIT_BYTE(&rt, code, TOY_OPCODE_RETURN); //temp terminator
	EMIT_BYTE(&rt, code, 0); //4-byte alignment
	EMIT_BYTE(&rt, code, 0);
	EMIT_BYTE(&rt, code, 0);

	//write the header and combine the parts
	void* buffer = NULL;
	unsigned int capacity = 0, count = 0;
	// int paramAddr = 0, codeAddr = 0, subsAddr = 0;
	int codeAddr = 0;
	int jumpsAddr = 0;
	int dataAddr = 0;

	emitInt(&buffer, &capacity, &count, 0); //total size (overwritten later)
	emitInt(&buffer, &capacity, &count, rt->paramCount); //param size
	emitInt(&buffer, &capacity, &count, rt->jumpsCount); //jumps size
	emitInt(&buffer, &capacity, &count, rt->dataCount); //data size
	emitInt(&buffer, &capacity, &count, rt->subsCount); //routine size

	//generate blank spaces, cache their positions in the *Addr variables (for storing the start positions)
	if (rt->paramCount > 0) {
		// paramAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //params
	}
	if (rt->codeCount > 0) {
		codeAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //code
	}
	if (rt->jumpsCount > 0) {
		jumpsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //jumps
	}
	if (rt->dataCount > 0) {
		dataAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //data
	}
	if (rt->subsCount > 0) {
		// subsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //subs
	}

	//append various parts to the buffer
	//TODO: param region

	if (rt->codeCount > 0) {
		expand(&buffer, &capacity, &count, rt->codeCount);
		memcpy((buffer + count), rt->code, rt->codeCount);

		*((int*)(buffer + codeAddr)) = count;
		count += rt->codeCount;
	}

	if (rt->jumpsCount > 0) {
		expand(&buffer, &capacity, &count, rt->jumpsCount);
		memcpy((buffer + count), rt->jumps, rt->jumpsCount);

		*((int*)(buffer + jumpsAddr)) = count;
		count += rt->jumpsCount;
	}

	if (rt->dataCount > 0) {
		expand(&buffer, &capacity, &count, rt->dataCount);
		memcpy((buffer + count), rt->data, rt->dataCount);

		*((int*)(buffer + dataAddr)) = count;
		count += rt->dataCount;
	}

	//TODO: subs region

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
	free(rt.param);
	free(rt.code);
	free(rt.jumps);
	free(rt.data);
	free(rt.subs);

	return buffer;
}
