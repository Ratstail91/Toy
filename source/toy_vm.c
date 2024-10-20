#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
#define READ_BYTE(vm) \
	vm->routine[vm->routineCounter++]

#define READ_UNSIGNED_INT(vm) \
	*((unsigned int*)(vm->routine + readPostfixUtil(&(vm->routineCounter), 4)))

#define READ_INT(vm) \
	*((int*)(vm->routine + readPostfixUtil(&(vm->routineCounter), 4)))

#define READ_FLOAT(vm) \
	*((float*)(vm->routine + readPostfixUtil(&(vm->routineCounter), 4)))

static inline int readPostfixUtil(unsigned int* ptr, int amount) {
	int ret = *ptr;
	*ptr += amount;
	return ret;
}

static inline void fixAlignment(Toy_VM* vm) {
	//NOTE: It's a tilde, not a negative sign
	vm->routineCounter = (vm->routineCounter + 3) & ~0b11;
}

//instruction handlers
static void processRead(Toy_VM* vm) {
	Toy_ValueType type = READ_BYTE(vm);

	Toy_Value value = TOY_VALUE_FROM_NULL();

	switch(type) {
		case TOY_VALUE_NULL: {
			//No-op
			break;
		}

		case TOY_VALUE_BOOLEAN: {
			value = TOY_VALUE_FROM_BOOLEAN((bool)READ_BYTE(vm));
			break;
		}

		case TOY_VALUE_INTEGER: {
			fixAlignment(vm);
			value = TOY_VALUE_FROM_INTEGER(READ_INT(vm));
			break;
		}

		case TOY_VALUE_FLOAT: {
			fixAlignment(vm);
			value = TOY_VALUE_FROM_FLOAT(READ_FLOAT(vm));
			break;
		}

		case TOY_VALUE_STRING: {
			fixAlignment(vm);
			//grab the jump as an integer
			unsigned int jump = *(unsigned int*)(vm->routine + vm->jumpsAddr + READ_INT(vm));

			//jumps are relative to the data address
			char* cstring = (char*)(vm->routine + vm->dataAddr + jump);

			//build a string from the data section
			value = TOY_VALUE_FROM_STRING(Toy_createString(&vm->stringBucket, cstring));

			break;
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
	fixAlignment(vm);
}

static void processDeclare(Toy_VM* vm) {
	Toy_ValueType type = READ_BYTE(vm); //variable type
	unsigned int len = READ_BYTE(vm); //name length
	fixAlignment(vm); //one spare byte

	//grab the jump
	unsigned int jump = *(unsigned int*)(vm->routine + vm->jumpsAddr + READ_INT(vm));

	//grab the data
	char* cstring = (char*)(vm->routine + vm->dataAddr + jump);

	//build the name string
	Toy_String* name = Toy_createNameStringLength(&vm->stringBucket, cstring, len, type);

	//get the value
	Toy_Value value = Toy_popStack(&vm->stack);

	//declare it
	Toy_declareScope(vm->scope, name, value);

	//cleanup
	Toy_freeString(name);
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
		left = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(left) );
	}
	else
	if (TOY_VALUE_IS_FLOAT(left) && TOY_VALUE_IS_INTEGER(right)) {
		right = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(right) );
	}

	//apply operation
	Toy_Value result = TOY_VALUE_FROM_NULL();

	if (opcode == TOY_OPCODE_ADD) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) + TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) + TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_SUBTRACT) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) - TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) - TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MULTIPLY) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) * TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) * TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_DIVIDE) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) / TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) / TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MODULO) {
		result = TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) % TOY_VALUE_AS_INTEGER(right) );
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processArithmetic, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}

	//finally
	Toy_pushStack(&vm->stack, result);
}

static void processComparison(Toy_VM* vm, Toy_OpcodeType opcode) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	//most things can be equal, so handle it separately
	if (opcode == TOY_OPCODE_COMPARE_EQUAL) {
		bool equal = TOY_VALUES_ARE_EQUAL(left, right);

		//equality has an optional "negate" opcode within it's word
		if (READ_BYTE(vm) != TOY_OPCODE_NEGATE) {
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(equal) );
		}
		else {
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(!equal) );
		}

		return;
	}

	//coerce ints into floats if needed
	if (TOY_VALUE_IS_INTEGER(left) && TOY_VALUE_IS_FLOAT(right)) {
		left = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(left) );
	}
	else
	if (TOY_VALUE_IS_FLOAT(left) && TOY_VALUE_IS_INTEGER(right)) {
		right = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(right) );
	}

	//other opcodes
	if (opcode == TOY_OPCODE_COMPARE_LESS) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_AS_FLOAT(left) < TOY_VALUE_AS_FLOAT(right) : TOY_VALUE_AS_INTEGER(left) < TOY_VALUE_AS_INTEGER(right)) );
	}
	else if (opcode == TOY_OPCODE_COMPARE_LESS_EQUAL) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_AS_FLOAT(left) <= TOY_VALUE_AS_FLOAT(right) : TOY_VALUE_AS_INTEGER(left) <= TOY_VALUE_AS_INTEGER(right)) );
	}
	else if (opcode == TOY_OPCODE_COMPARE_GREATER) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_AS_FLOAT(left) > TOY_VALUE_AS_FLOAT(right) : TOY_VALUE_AS_INTEGER(left) > TOY_VALUE_AS_INTEGER(right)) );
	}
	else if (opcode == TOY_OPCODE_COMPARE_GREATER_EQUAL) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_AS_FLOAT(left) >= TOY_VALUE_AS_FLOAT(right) : TOY_VALUE_AS_INTEGER(left) >= TOY_VALUE_AS_INTEGER(right)) );
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processComparison, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}
}

static void processLogical(Toy_VM* vm, Toy_OpcodeType opcode) {
	if (opcode == TOY_OPCODE_AND) {
		Toy_Value right = Toy_popStack(&vm->stack);
		Toy_Value left = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( TOY_VALUE_IS_TRUTHY(left) && TOY_VALUE_IS_TRUTHY(right) ));
	}
	else if (opcode == TOY_OPCODE_OR) {
		Toy_Value right = Toy_popStack(&vm->stack);
		Toy_Value left = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( TOY_VALUE_IS_TRUTHY(left) || TOY_VALUE_IS_TRUTHY(right) ));
	}
	else if (opcode == TOY_OPCODE_TRUTHY) {
		Toy_Value top = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( TOY_VALUE_IS_TRUTHY(top) ));
	}
	else if (opcode == TOY_OPCODE_NEGATE) {
		Toy_Value top = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( !TOY_VALUE_IS_TRUTHY(top) ));
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processLogical, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}
}

static void processPrint(Toy_VM* vm) {
	//print the value on top of the stack, popping it
	Toy_Value value = Toy_popStack(&vm->stack);

	//NOTE: don't append a newline - leave that choice to the host
	switch(value.type) {
		case TOY_VALUE_NULL:
			Toy_print("null");
			break;

		case TOY_VALUE_BOOLEAN:
			Toy_print(TOY_VALUE_AS_BOOLEAN(value) ? "true" : "false");
			break;

		case TOY_VALUE_INTEGER: {
			char buffer[16];
			sprintf(buffer, "%d", TOY_VALUE_AS_INTEGER(value));
			Toy_print(buffer);
			break;
		}

		case TOY_VALUE_FLOAT: {
			char buffer[16];
			sprintf(buffer, "%f", TOY_VALUE_AS_FLOAT(value));
			Toy_print(buffer);
			break;
		}

		case TOY_VALUE_STRING: {
			Toy_String* str = TOY_VALUE_AS_STRING(value);

			//TODO: decide on how long strings, etc. live for in memory
			if (str->type == TOY_STRING_NODE) {
				char* buffer = Toy_getStringRawBuffer(str);
				Toy_print(buffer);
				free(buffer);
			}
			else if (str->type == TOY_STRING_LEAF) {
				Toy_print(str->as.leaf.data);
			}
			else if (str->type == TOY_STRING_NAME) {
				Toy_print(str->as.name.data); //should this be a thing?
			}
			break;
		}

		case TOY_VALUE_ARRAY:
		case TOY_VALUE_DICTIONARY:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown value type %d passed to processPrint, exiting\n" TOY_CC_RESET, value.type);
			exit(-1);
	}
}

static void processConcat(Toy_VM* vm) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_STRING(left)) {
		Toy_error("Failed to concatenate a value that is not a string");
		return;
	}

	if (!TOY_VALUE_IS_STRING(left)) {
		Toy_error("Failed to concatenate a value that is not a string");
		return;
	}

	//all good
	Toy_String* result = Toy_concatStrings(&vm->stringBucket, TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right));
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_STRING(result));
}

static void process(Toy_VM* vm) {
	while(true) {
		Toy_OpcodeType opcode = READ_BYTE(vm);

		switch(opcode) {
			//variable instructions
			case TOY_OPCODE_READ:
				processRead(vm);
				break;

			case TOY_OPCODE_DECLARE:
				processDeclare(vm);
				break;

			//arithmetic instructions
			case TOY_OPCODE_ADD:
			case TOY_OPCODE_SUBTRACT:
			case TOY_OPCODE_MULTIPLY:
			case TOY_OPCODE_DIVIDE:
			case TOY_OPCODE_MODULO:
				processArithmetic(vm, opcode);
				break;

			//comparison instructions
			case TOY_OPCODE_COMPARE_EQUAL:
			case TOY_OPCODE_COMPARE_LESS:
			case TOY_OPCODE_COMPARE_LESS_EQUAL:
			case TOY_OPCODE_COMPARE_GREATER:
			case TOY_OPCODE_COMPARE_GREATER_EQUAL:
				processComparison(vm, opcode);
				break;

			//logical instructions
			case TOY_OPCODE_AND:
			case TOY_OPCODE_OR:
			case TOY_OPCODE_TRUTHY:
			case TOY_OPCODE_NEGATE:
				processLogical(vm, opcode);
				break;

			//control instructions
			case TOY_OPCODE_RETURN:
				//temp terminator
				return;

			//various action instructions
			case TOY_OPCODE_PRINT:
				processPrint(vm);
				break;

			case TOY_OPCODE_CONCAT:
				processConcat(vm);
				break;

			//not yet implemented
			case TOY_OPCODE_ASSIGN:
			case TOY_OPCODE_ACCESS:
				fprintf(stderr, TOY_CC_ERROR "ERROR: Incomplete opcode %d found, exiting\n" TOY_CC_RESET, opcode);
				exit(-1);

			case TOY_OPCODE_PASS:
			case TOY_OPCODE_ERROR:
			case TOY_OPCODE_EOF:
				fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d found, exiting\n" TOY_CC_RESET, opcode);
				exit(-1);
		}

		//prepare for the next instruction
		fixAlignment(vm);
	}
}

//exposed functions
void Toy_initVM(Toy_VM* vm) {
	//clear the stack, scope and memory
	vm->stringBucket = NULL;
	vm->scopeBucket = NULL;
	vm->stack = NULL;
	vm->scope = NULL;

	Toy_resetVM(vm);
}

void Toy_bindVM(Toy_VM* vm, unsigned char* bytecode) {
	if (bytecode[0] != TOY_VERSION_MAJOR || bytecode[1] > TOY_VERSION_MINOR) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Wrong bytecode version found: expected %d.%d.%d found %d.%d.%d, exiting\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, bytecode[0], bytecode[1], bytecode[2]);
		exit(-1);
	}

	if (bytecode[2] != TOY_VERSION_PATCH) {
		fprintf(stderr, TOY_CC_WARN "WARNING: Wrong bytecode version found: expected %d.%d.%d found %d.%d.%d, continuing\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, bytecode[0], bytecode[1], bytecode[2]);
	}

	if (strcmp((char*)(bytecode + 3), TOY_VERSION_BUILD) != 0) {
		fprintf(stderr, TOY_CC_WARN "WARNING: Wrong bytecode build info found: expected '%s' found '%s', continuing\n" TOY_CC_RESET, TOY_VERSION_BUILD, (char*)(bytecode + 3));
	}

	//offset by the header size
	int offset = 3 + strlen(TOY_VERSION_BUILD) + 1;
	if (offset % 4 != 0) {
		offset += 4 - (offset % 4); //ceil
	}

	//delegate
	Toy_bindVMToRoutine(vm, bytecode + offset);

	//cache these
	vm->bc = bytecode;
}

void Toy_bindVMToRoutine(Toy_VM* vm, unsigned char* routine) {
	vm->routine = routine;

	//read the header metadata
	vm->routineSize = READ_UNSIGNED_INT(vm);
	vm->paramSize = READ_UNSIGNED_INT(vm);
	vm->jumpsSize = READ_UNSIGNED_INT(vm);
	vm->dataSize = READ_UNSIGNED_INT(vm);
	vm->subsSize = READ_UNSIGNED_INT(vm);

	//read the header addresses
	if (vm->paramSize > 0) {
		vm->paramAddr = READ_UNSIGNED_INT(vm);
	}

	vm->codeAddr = READ_UNSIGNED_INT(vm); //required

	if (vm->jumpsSize > 0) {
		vm->jumpsAddr = READ_UNSIGNED_INT(vm);
	}

	if (vm->dataSize > 0) {
		vm->dataAddr = READ_UNSIGNED_INT(vm);
	}

	if (vm->subsSize > 0) {
		vm->subsAddr = READ_UNSIGNED_INT(vm);
	}

	//allocate the stack, scope, and memory
	vm->stringBucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
	vm->scopeBucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
	vm->stack = Toy_allocateStack();
	vm->scope = Toy_pushScope(&vm->scopeBucket, NULL);
}

void Toy_runVM(Toy_VM* vm) {
	//TODO: read params into scope

	//prep the routine counter for execution
	vm->routineCounter = vm->codeAddr;

	//begin
	process(vm);
}

void Toy_freeVM(Toy_VM* vm) {
	//clear the stack, scope and memory
	Toy_freeStack(vm->stack);
	Toy_popScope(vm->scope);
	Toy_freeBucket(&vm->stringBucket);
	Toy_freeBucket(&vm->scopeBucket);

	//free the bytecode
	free(vm->bc);

	Toy_resetVM(vm);
}

void Toy_resetVM(Toy_VM* vm) {
	vm->bc = NULL;

	vm->routine = NULL;
	vm->routineSize = 0;

	vm->paramSize = 0;
	vm->jumpsSize = 0;
	vm->dataSize = 0;
	vm->subsSize = 0;

	vm->paramAddr = 0;
	vm->codeAddr = 0;
	vm->jumpsAddr = 0;
	vm->dataAddr = 0;
	vm->subsAddr = 0;

	vm->routineCounter = 0;

	//NOTE: stack, scope and memory are not altered during resets
}
