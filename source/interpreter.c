#include "interpreter.h"
#include "console_colors.h"

#include "common.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>

static void stdoutWrapper(const char* output) {
	fprintf(stdout, output);
	fprintf(stdout, "\n"); //default new line
}

static void stderrWrapper(const char* output) {
	fprintf(stderr, "Assertion failure: ");
	fprintf(stderr, output);
	fprintf(stderr, "\n"); //default new line
}

void initInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length) {
	initLiteralArray(&interpreter->literalCache);
	interpreter->scope = pushScope(NULL);
	interpreter->bytecode = bytecode;
	interpreter->length = length;
	interpreter->count = 0;

	initLiteralArray(&interpreter->stack);

	setInterpreterPrint(interpreter, stdoutWrapper);
	setInterpreterAssert(interpreter, stderrWrapper);
}

void freeInterpreter(Interpreter* interpreter) {
	FREE_ARRAY(char, interpreter->bytecode, interpreter->length);

	//since these are dynamically allocated, free them manually
	for (int i = 0; i < interpreter->literalCache.count; i++) {
		if (IS_ARRAY(interpreter->literalCache.literals[i]) || IS_DICTIONARY(interpreter->literalCache.literals[i]) || IS_TYPE(interpreter->literalCache.literals[i])) {

			if (IS_TYPE(interpreter->literalCache.literals[i]) && AS_TYPE(interpreter->literalCache.literals[i]).capacity > 0) {
				FREE_ARRAY(Literal, AS_TYPE(interpreter->literalCache.literals[i]).subtypes, AS_TYPE(interpreter->literalCache.literals[i]).capacity);
			}

			freeLiteral(interpreter->literalCache.literals[i]);

			interpreter->literalCache.literals[i] = TO_NULL_LITERAL;
		}
	}
	freeLiteralArray(&interpreter->literalCache);

	while (interpreter->scope) {
		interpreter->scope = popScope(interpreter->scope);
	}

	freeLiteralArray(&interpreter->stack);
}

//utilities for the host program
void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput) {
	interpreter->printOutput = printOutput;
}

void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput) {
	interpreter->assertOutput = assertOutput;
}

//utils
static unsigned char readByte(unsigned char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	*count += 1;
	return ret;
}

static unsigned short readShort(unsigned char* tb, int* count) {
	unsigned short ret = *(unsigned short*)(tb + *count);
	*count += 2;
	return ret;
}

static int readInt(unsigned char* tb, int* count) {
	int ret = *(int*)(tb + *count);
	*count += 4;
	return ret;
}

static float readFloat(unsigned char* tb, int* count) {
	float ret = *(float*)(tb + *count);
	*count += 4;
	return ret;
}

static char* readString(unsigned char* tb, int* count) {
	unsigned char* ret = tb + *count;
	*count += strlen((char*)ret) + 1; //+1 for null character
	return (char*)ret;
}

static void consumeByte(unsigned char byte, unsigned char* tb, int* count) {
	if (byte != tb[*count]) {
		printf("[internal] Failed to consume the correct byte  (expected %u, found %u)\n", byte, tb[*count]);
	}
	*count += 1;
}

static void consumeShort(unsigned short bytes, unsigned char* tb, int* count) {
	if (bytes != *(unsigned short*)(tb + *count)) {
		printf("[internal] Failed to consume the correct bytes (expected %u, found %u)\n", bytes, *(unsigned short*)(tb + *count));
	}
	*count += 2;
}

//each available statement
static bool execAssert(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (!IS_STRING(rhs)) {
		printf("[internal] The interpreter's assert keyword needs a string as the second argument, received: ");
		printLiteral(rhs);
		printf("\n");
		return false;
	}

	if (!IS_TRUTHY(lhs)) {
		(*interpreter->assertOutput)(AS_STRING(rhs));
		return false;
	}

	return true;
}

static bool execPrint(Interpreter* interpreter) {
	//print what is on top of the stack, then pop it
	Literal lit = popLiteralArray(&interpreter->stack);

	printLiteralCustom(lit, interpreter->printOutput);

	freeLiteral(lit);

	return true;
}

static bool execPushLiteral(Interpreter* interpreter, bool lng) {
	//read the index in the cache
	int index = 0;

	if (lng) {
		index = (int)readShort(interpreter->bytecode, &interpreter->count);
	}
	else {
		index = (int)readByte(interpreter->bytecode, &interpreter->count);
	}

	//push from cache to stack
	pushLiteralArray(&interpreter->stack, interpreter->literalCache.literals[index]);

	return true;
}

static bool execNegate(Interpreter* interpreter) {
	//negate the top literal on the stack
	Literal lit = popLiteralArray(&interpreter->stack);

	if (IS_INTEGER(lit)) {
		lit = TO_INTEGER_LITERAL(-AS_INTEGER(lit));
	}
	else if (IS_FLOAT(lit)) {
		lit = TO_FLOAT_LITERAL(-AS_FLOAT(lit));
	}
	else {
		printf("[internal] The interpreter can't negate that literal: ");
		printLiteral(lit);
		printf("\n");
		return false;
	}

	pushLiteralArray(&interpreter->stack, lit);
	return true;
}

static bool execArithmetic(Interpreter* interpreter, Opcode opcode) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	//type coersion
	if (IS_FLOAT(lhs) && IS_INTEGER(rhs)) {
		rhs = TO_FLOAT_LITERAL(AS_INTEGER(rhs));
	}

	if (IS_INTEGER(lhs) && IS_FLOAT(rhs)) {
		lhs = TO_FLOAT_LITERAL(AS_INTEGER(lhs));
	}

	//maths based on types
	if(IS_INTEGER(lhs) && IS_INTEGER(rhs)) {
		switch(opcode) {
			case OP_ADDITION:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) + AS_INTEGER(rhs) ));
				return true;

			case OP_SUBTRACTION:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) - AS_INTEGER(rhs) ));
				return true;

			case OP_MULTIPLICATION:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) * AS_INTEGER(rhs) ));
				return true;

			case OP_DIVISION:
				if (AS_INTEGER(rhs) == 0) {
					printf("Can't divide by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) / AS_INTEGER(rhs) ));
				return true;

			case OP_MODULO:
				if (AS_INTEGER(rhs) == 0) {
					printf("Can't modulo by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) % AS_INTEGER(rhs) ));
				return true;

			default:
				printf("[internal] bad opcode argument passed to execArithmetic()");
				return false;
		}
	}

	//catch bad modulo
	if (opcode == OP_MODULO) {
		printf("Bad arithmetic argument (modulo on floats not allowed)\n");
		return false;
	}

	if(IS_FLOAT(lhs) && IS_FLOAT(rhs)) {
		switch(opcode) {
			case OP_ADDITION:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) + AS_FLOAT(rhs) ));
				return true;

			case OP_SUBTRACTION:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) - AS_FLOAT(rhs) ));
				return true;

			case OP_MULTIPLICATION:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) * AS_FLOAT(rhs) ));
				return true;

			case OP_DIVISION:
				if (AS_FLOAT(rhs) == 0) {
					printf("Can't divide by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) / AS_FLOAT(rhs) ));
				return true;

			default:
				printf("[internal] bad opcode argument passed to execArithmetic()");
				return false;
		}
	}

	//wrong types
	printf("Bad arithmetic argument\n");
	return false;
}

static bool execVarDecl(Interpreter* interpreter, bool lng) {
	//read the index in the cache
	int identifierIndex = 0;
	int typeIndex = 0;

	if (lng) {
		identifierIndex = (int)readShort(interpreter->bytecode, &interpreter->count);
		typeIndex = (int)readShort(interpreter->bytecode, &interpreter->count);
	}
	else {
		identifierIndex = (int)readByte(interpreter->bytecode, &interpreter->count);
		typeIndex = (int)readByte(interpreter->bytecode, &interpreter->count);
	}

	Literal identifier = interpreter->literalCache.literals[identifierIndex];
	Literal type = interpreter->literalCache.literals[typeIndex];

	if (!declareScopeVariable(interpreter->scope, identifier, type)) {
		return false;
	}

	if (!setScopeVariable(interpreter->scope, identifier, popLiteralArray(&interpreter->stack) )) {
		return false;
	}

	return true;
}

//the heart of toy
static void execInterpreter(Interpreter* interpreter) {
	unsigned char opcode = readByte(interpreter->bytecode, &interpreter->count);

	while(opcode != OP_EOF && opcode != OP_SECTION_END) {
		switch(opcode) {
			case OP_ASSERT:
				if (!execAssert(interpreter)) {
					return;
				}
			break;

			case OP_PRINT:
				if (!execPrint(interpreter)) {
					return;
				}
			break;

			case OP_LITERAL:
			case OP_LITERAL_LONG:
				if (!execPushLiteral(interpreter, opcode == OP_LITERAL_LONG)) {
					return;
				}
			break;

			case OP_NEGATE:
				if (!execNegate(interpreter)) {
					return;
				}
			break;

			case OP_ADDITION:
			case OP_SUBTRACTION:
			case OP_MULTIPLICATION:
			case OP_DIVISION:
			case OP_MODULO:
				if (!execArithmetic(interpreter, opcode)) {
					return;
				}
			break;

			case OP_GROUPING_BEGIN:
				execInterpreter(interpreter);
			break;

			case OP_GROUPING_END:
				return;

			//scope
			case OP_SCOPE_BEGIN:
				interpreter->scope = pushScope(interpreter->scope);
			break;

			case OP_SCOPE_END:
				interpreter->scope = popScope(interpreter->scope);
			break;

			//TODO: type declarations

			case OP_VAR_DECL:
			case OP_VAR_DECL_LONG:
				if (!execVarDecl(interpreter, opcode == OP_LITERAL_LONG)) {
					return;
				}
			break;

			default:
				printf("Unknown opcode found %d, terminating\n", opcode);
				printLiteralArray(&interpreter->stack, "\n");
				return;
		}

		opcode = readByte(interpreter->bytecode, &interpreter->count);
	}
}

void runInterpreter(Interpreter* interpreter) {
	if (!interpreter->bytecode) {
		printf(ERROR "Error: No valid bytecode given\n" RESET);
		return;
	}

	//header section
	const unsigned char major = readByte(interpreter->bytecode, &interpreter->count);
	const unsigned char minor = readByte(interpreter->bytecode, &interpreter->count);
	const unsigned char patch = readByte(interpreter->bytecode, &interpreter->count);

	if (major != TOY_VERSION_MAJOR || minor != TOY_VERSION_MINOR || patch != TOY_VERSION_PATCH) {
		printf(ERROR "Error: interpreter/bytecode version mismatch\n" RESET);
	}

	const char* build = readString(interpreter->bytecode, &interpreter->count);

	if (command.verbose) {
		if (strncmp(build, TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD))) {
			printf(WARN "Warning: interpreter/bytecode build mismatch\n" RESET);
		}
	}

	consumeByte(OP_SECTION_END, interpreter->bytecode, &interpreter->count);

	//data section
	const short literalCount = readShort(interpreter->bytecode, &interpreter->count);

	if (command.verbose) {
		printf(NOTICE "Reading %d literals\n" RESET, literalCount);
	}

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = readByte(interpreter->bytecode, &interpreter->count);

		switch(literalType) {
			case LITERAL_NULL:
				//read the null
				pushLiteralArray(&interpreter->literalCache, TO_NULL_LITERAL);

				if (command.verbose) {
					printf("(null)\n");
				}
			break;

			case LITERAL_BOOLEAN: {
				//read the booleans
				const bool b = readByte(interpreter->bytecode, &interpreter->count);
				pushLiteralArray(&interpreter->literalCache, TO_BOOLEAN_LITERAL(b));

				if (command.verbose) {
					printf("(boolean %s)\n", b ? "true" : "false");
				}
			}
			break;

			case LITERAL_INTEGER: {
				const int d = readInt(interpreter->bytecode, &interpreter->count);
				pushLiteralArray(&interpreter->literalCache, TO_INTEGER_LITERAL(d));

				if (command.verbose) {
					printf("(integer %d)\n", d);
				}
			}
			break;

			case LITERAL_FLOAT: {
				const float f = readFloat(interpreter->bytecode, &interpreter->count);
				pushLiteralArray(&interpreter->literalCache, TO_FLOAT_LITERAL(f));

				if (command.verbose) {
					printf("(float %f)\n", f);
				}
			}
			break;

			case LITERAL_STRING: {
				char* s = readString(interpreter->bytecode, &interpreter->count);
				pushLiteralArray(&interpreter->literalCache, TO_STRING_LITERAL(s));

				if (command.verbose) {
					printf("(string \"%s\")\n", s);
				}
			}
			break;

			case LITERAL_ARRAY: {
				LiteralArray* array = ALLOCATE(LiteralArray, 1);
				initLiteralArray(array);

				unsigned short length = readShort(interpreter->bytecode, &interpreter->count);

				//read each index, then unpack the value from the existing literal cache
				for (int i = 0; i < length; i++) {
					int index = readShort(interpreter->bytecode, &interpreter->count);
					pushLiteralArray(array, interpreter->literalCache.literals[index]);
				}

				if (command.verbose) {
					printf("(array ");
					printLiteral(TO_ARRAY_LITERAL(array));
					printf(")\n");
				}

				//finally, push the array proper
				pushLiteralArray(&interpreter->literalCache, TO_ARRAY_LITERAL(array));
			}
			break;

			case LITERAL_DICTIONARY: {
				LiteralDictionary* dictionary = ALLOCATE(LiteralDictionary, 1);
				initLiteralDictionary(dictionary);

				unsigned short length = readShort(interpreter->bytecode, &interpreter->count);

				//read each index, then unpack the value from the existing literal cache
				for (int i = 0; i < length / 2; i++) {
					int key = readShort(interpreter->bytecode, &interpreter->count);
					int val = readShort(interpreter->bytecode, &interpreter->count);
					setLiteralDictionary(dictionary, interpreter->literalCache.literals[key], interpreter->literalCache.literals[val]);
				}

				if (command.verbose) {
					printf("(dictionary ");
					printLiteral(TO_DICTIONARY_LITERAL(dictionary));
					printf(")\n");
				}

				//finally, push the dictionary proper
				pushLiteralArray(&interpreter->literalCache, TO_DICTIONARY_LITERAL(dictionary));
			}
			break;

			//TODO: functions

			case LITERAL_IDENTIFIER: {
				char* str = readString(interpreter->bytecode, &interpreter->count);

				Literal identifier = TO_IDENTIFIER_LITERAL(str);

				pushLiteralArray(&interpreter->literalCache, identifier);

				if (command.verbose) {
					printf("(identifier %s (%d))\n", AS_IDENTIFIER(identifier), identifier.as.identifier.hash);
				}
			}
			break;

			case LITERAL_TYPE: {
				Literal typeLiteral;

				//read the array count (subtract 1, because mask is always present)
				unsigned short count = readShort(interpreter->bytecode, &interpreter->count) - 1;

				// read the mask
				unsigned char mask = readShort(interpreter->bytecode, &interpreter->count);

				//create the literal
				typeLiteral = TO_TYPE_LITERAL(mask);

				//if it's got subtypes, grab them from the existing cache
				if (count > 0) {
					AS_TYPE(typeLiteral).subtypes = ALLOCATE(Literal, count);
					AS_TYPE(typeLiteral).capacity = count;
					AS_TYPE(typeLiteral).count = count;

					for (int i = 0; i < AS_TYPE(typeLiteral).count; i++) {
						//read each index
						int index = readShort(interpreter->bytecode, &interpreter->count);
						((Literal*)(AS_TYPE(typeLiteral).subtypes))[i] = interpreter->literalCache.literals[index];
					}
				}

				//save the type
				pushLiteralArray(&interpreter->literalCache, typeLiteral);

				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}
			}
			break;
		}
	}

	consumeByte(OP_SECTION_END, interpreter->bytecode, &interpreter->count);

	//code section
	if (command.verbose) {
		printf(NOTICE "executing bytecode\n" RESET);
	}

	execInterpreter(interpreter);
}
