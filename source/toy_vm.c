#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_memory.h"
#include "toy_opcodes.h"
#include "toy_value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
#define READ_BYTE(vm) \
	vm->program[vm->programCounter++]

#define READ_INT(vm) \
	*((int*)(vm->program + _read_postfix(&(vm->programCounter), 4)))

#define READ_FLOAT(vm) \
	*((float*)(vm->program + _read_postfix(&(vm->programCounter), 4)))

static inline int _read_postfix(int* ptr, int amount) {
	int ret = *ptr;
	*ptr += amount;
	return ret;
}

static inline void fix_alignment(Toy_VM* vm) {
	if (vm->programCounter % 4 != 0) {
		vm->programCounter = (4 - vm->programCounter % 4);
	}
}

//instruction handlers
static void processRead(Toy_VM* vm) {
	Toy_ValueType type = READ_BYTE(vm);

	Toy_Value value = TOY_VALUE_TO_NULL();

	switch(type) {
		case TOY_VALUE_NULL: {
			//No-op
			break;
		}

		case TOY_VALUE_BOOLEAN: {
			value = TOY_VALUE_TO_BOOLEAN((bool)READ_BYTE(vm));
			break;
		}

		case TOY_VALUE_INTEGER: {
			fix_alignment(vm);
			value = TOY_VALUE_TO_INTEGER(READ_INT(vm));
			break;
		}

		case TOY_VALUE_FLOAT: {
			fix_alignment(vm);
			value = TOY_VALUE_TO_FLOAT(READ_FLOAT(vm));
			break;
		}

		case TOY_VALUE_STRING: {
			//
			// break;
		}

		case TOY_VALUE_ARRAY: {
			//
			// break;
		}

		case TOY_VALUE_DICTIONARY: {
			//
			// break;
		}

		case TOY_VALUE_FUNCTION: {
			//
			// break;
		}

		case TOY_VALUE_OPAQUE: {
			//
			// break;
		}

		default:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid value type %d found, exiting\n" TOY_CC_RESET, type);
			exit(-1);
	}

	//push onto the stack
	Toy_pushStack(&vm->stack, value);

	//leave the counter in a good spot
	fix_alignment(vm);
}

static void processArithmetic(Toy_VM* vm, Toy_OpcodeType opcode) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	//check types
	if ((!TOY_VALUE_IS_INTEGER(left) && !TOY_VALUE_IS_FLOAT(left)) || (!TOY_VALUE_IS_INTEGER(right) && !TOY_VALUE_IS_FLOAT(right))) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid types %d and %d passed to processArithmetic, exiting\n" TOY_CC_RESET, left.type, right.type);
		exit(-1);
	}

	//check for divide by zero
	if (opcode == TOY_OPCODE_DIVIDE || opcode == TOY_OPCODE_MODULO) {
		if ((TOY_VALUE_IS_INTEGER(right) && TOY_VALUE_AS_INTEGER(right) == 0) || (TOY_VALUE_IS_FLOAT(right) && TOY_VALUE_AS_FLOAT(right) == 0)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't divide by zero, exiting\n" TOY_CC_RESET);
			exit(-1);
		}
	}

	//check for modulo by a float
	if (opcode == TOY_OPCODE_MODULO && TOY_VALUE_IS_FLOAT(right)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't modulo by a float, exiting\n" TOY_CC_RESET);
		exit(-1);
	}

	//coerce ints into floats if needed
	if (TOY_VALUE_IS_INTEGER(left) && TOY_VALUE_IS_FLOAT(right)) {
		left = TOY_VALUE_TO_FLOAT( (float)TOY_VALUE_AS_INTEGER(left) );
	}
	else
	if (TOY_VALUE_IS_FLOAT(left) && TOY_VALUE_IS_INTEGER(right)) {
		right = TOY_VALUE_TO_FLOAT( (float)TOY_VALUE_AS_INTEGER(right) );
	}

	//apply operation
	Toy_Value result = TOY_VALUE_TO_NULL();

	if (opcode == TOY_OPCODE_ADD) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_TO_FLOAT( TOY_VALUE_AS_FLOAT(left) + TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_TO_INTEGER( TOY_VALUE_AS_INTEGER(left) + TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_SUBTRACT) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_TO_FLOAT( TOY_VALUE_AS_FLOAT(left) - TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_TO_INTEGER( TOY_VALUE_AS_INTEGER(left) - TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MULTIPLY) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_TO_FLOAT( TOY_VALUE_AS_FLOAT(left) * TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_TO_INTEGER( TOY_VALUE_AS_INTEGER(left) * TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_DIVIDE) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_TO_FLOAT( TOY_VALUE_AS_FLOAT(left) / TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_TO_INTEGER( TOY_VALUE_AS_INTEGER(left) / TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MODULO) {
		result = TOY_VALUE_TO_INTEGER( TOY_VALUE_AS_INTEGER(left) % TOY_VALUE_AS_INTEGER(right) );
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processArithmetic, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}

	//finally
	Toy_pushStack(&vm->stack, result);
}

static void process(Toy_VM* vm) {
	Toy_OpcodeType opcode = READ_BYTE(vm);

	switch(opcode) {
		case TOY_OPCODE_READ:
			processRead(vm);
			break;

		case TOY_OPCODE_ADD:
		case TOY_OPCODE_SUBTRACT:
		case TOY_OPCODE_MULTIPLY:
		case TOY_OPCODE_DIVIDE:
		case TOY_OPCODE_MODULO:
			processArithmetic(vm, opcode);
			break;

		case TOY_OPCODE_COMPARE_EQUAL:
			//
			// break;

		case TOY_OPCODE_COMPARE_LESS:
			//
			// break;

		case TOY_OPCODE_COMPARE_LESS_EQUAL:
			//
			// break;

		case TOY_OPCODE_COMPARE_GREATER:
			//
			// break;

		case TOY_OPCODE_COMPARE_GREATER_EQUAL:
			//
			// break;

		case TOY_OPCODE_AND:
			//
			// break;

		case TOY_OPCODE_OR:
			//
			// break;

		case TOY_OPCODE_TRUTHY:
			//
			// break;

		case TOY_OPCODE_NEGATE: //TODO: squeeze into !=
			//
			// break;

		case TOY_OPCODE_LOAD:
		case TOY_OPCODE_LOAD_LONG:
		case TOY_OPCODE_DECLARE:
		case TOY_OPCODE_ASSIGN:
		case TOY_OPCODE_ACCESS:
		case TOY_OPCODE_PASS:
		case TOY_OPCODE_ERROR:
		case TOY_OPCODE_EOF:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d found, exiting\n" TOY_CC_RESET, opcode);
			exit(-1);

		case TOY_OPCODE_RETURN: //temp terminator, temp position
			//
			// return;
	}
}

//exposed functions
void Toy_initVM(Toy_VM* vm) {
	vm->program = NULL;
	vm->programSize = 0;

	vm->paramCount = 0;
	vm->jumpsCount = 0;
	vm->dataCount = 0;
	vm->subsCount = 0;

	vm->paramAddr = 0;
	vm->codeAddr = 0;
	vm->jumpsAddr = 0;
	vm->dataAddr = 0;
	vm->subsAddr = 0;

	vm->programCounter = 0;

	//init the scope & stack
	Toy_initStack(&vm->stack);
}

void Toy_bindVM(Toy_VM* vm, unsigned char* program) {
	vm->program = program;

	//read the header metadata
	vm->programSize = READ_INT(vm);
	vm->paramCount = READ_INT(vm);
	vm->jumpsCount = READ_INT(vm);
	vm->dataCount = READ_INT(vm);
	vm->subsCount = READ_INT(vm);

	//read the header addresses
	if (vm->paramCount > 0) {
		vm->paramAddr = READ_INT(vm);
	}

	vm->codeAddr = READ_INT(vm);

	if (vm->jumpsCount > 0) {
		vm->jumpsAddr = READ_INT(vm);
	}

	if (vm->dataCount > 0) {
		vm->dataAddr = READ_INT(vm);
	}

	if (vm->subsCount > 0) {
		vm->subsAddr = READ_INT(vm);
	}

	//preallocate the scope & stack
	Toy_preallocateStack(&vm->stack);
}

void Toy_runVM(Toy_VM* vm) {
	//TODO: read params into scope

	//prep the program counter for execution
	vm->programCounter = vm->codeAddr;

	//begin
	process(vm);
}

void Toy_freeVM(Toy_VM* vm) {
	//clear the stack
	Toy_freeStack(&vm->stack);

	//TODO: clear the scope

	//free the bytecode
	TOY_FREE_ARRAY(unsigned char, vm->program, vm->programSize);
	Toy_initVM(vm);
}
