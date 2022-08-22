#include "interpreter.h"
#include "console_colors.h"

#include "common.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>

static void stdoutWrapper(const char* output) {
	fprintf(stdout, "%s", output);
	fprintf(stdout, "\n"); //default new line
}

static void stderrWrapper(const char* output) {
	fprintf(stderr, "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n"); //default new line
}

void initInterpreter(Interpreter* interpreter) {
	initLiteralArray(&interpreter->literalCache);
	interpreter->scope = pushScope(NULL);
	interpreter->bytecode = NULL;
	interpreter->length = 0;
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

static bool parseIdentifierToValue(Interpreter* interpreter, Literal* literalPtr) {
	//this converts identifiers to values
	if (IS_IDENTIFIER(*literalPtr)) {
		if (!getScopeVariable(interpreter->scope, *literalPtr, literalPtr)) {
			printf("Undeclared variable \"");;
			printLiteral(*literalPtr);
			printf("\"\n");
			return false;
		}
	}

	return true;
}

//each available statement
static bool execAssert(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);
	parseIdentifierToValue(interpreter, &lhs);

	if (!IS_STRING(rhs)) {
		printf("The assert keyword needs a string as the second argument, received: ");
		printLiteral(rhs);
		printf("\n");
		return false;
	}

	if (IS_NULL(lhs) || !IS_TRUTHY(lhs)) {
		(*interpreter->assertOutput)(AS_STRING(rhs));
		return false;
	}

	return true;
}

static bool execPrint(Interpreter* interpreter) {
	//print what is on top of the stack, then pop it
	Literal lit = popLiteralArray(&interpreter->stack);
	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

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

	//push from cache to stack (DO NOT account for identifiers - will do that later)
	pushLiteralArray(&interpreter->stack, interpreter->literalCache.literals[index]);

	return true;
}

static bool rawLiteral(Interpreter* interpreter) {
	Literal lit = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	pushLiteralArray(&interpreter->stack, lit);

	return true;
}

static bool execNegate(Interpreter* interpreter) {
	//negate the top literal on the stack (numbers only)
	Literal lit = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	else if (IS_INTEGER(lit)) {
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

static bool execInvert(Interpreter* interpreter) {
	//negate the top literal on the stack (booleans only)
	Literal lit = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	if (IS_BOOLEAN(lit)) {
		lit = TO_BOOLEAN_LITERAL(!AS_BOOLEAN(lit));
	}
	else {
		printf("[internal] The interpreter can't invert that literal: ");
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

	parseIdentifierToValue(interpreter, &rhs);
	parseIdentifierToValue(interpreter, &lhs);

	//special case for string concatenation ONLY
	if (IS_STRING(lhs) && IS_STRING(rhs)) {
		//check for overflow
		if (STRLEN(lhs) + STRLEN(rhs) > MAX_STRING_LENGTH) {
			printf("Can't concatenate these strings (result is too long)\n");
			return false;
		}

		//concat the strings
		char buffer[MAX_STRING_LENGTH];
		snprintf(buffer, MAX_STRING_LENGTH, "%s%s", AS_STRING(lhs), AS_STRING(rhs));
		pushLiteralArray(&interpreter->stack, TO_STRING_LITERAL(buffer));

		return true;
	}

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
	printf("Bad arithmetic argument ");
	printLiteral(lhs);
	printf(" and ");
	printLiteral(rhs);
	printf("\n");
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

	parseIdentifierToValue(interpreter, &type);

	if (!declareScopeVariable(interpreter->scope, identifier, type)) {
		printf("Can't redefine the variable \"");
		printLiteral(identifier);
		printf("\"\n");
		return false;
	}

	Literal val = popLiteralArray(&interpreter->stack);
	parseIdentifierToValue(interpreter, &val);

	if (!IS_NULL(val) && !setScopeVariable(interpreter->scope, identifier, val, false)) {
		printf("Incorrect type assigned to variable \"");
		printLiteral(identifier);
		printf("\"\n");
		return false;
	}

	return true;
}

static bool execVarAssign(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	parseIdentifierToValue(interpreter, &rhs);

	if (!IS_IDENTIFIER(lhs)) {
		printf("Can't assign to a non-variable \"");
		printLiteral(lhs);
		printf("\"\n");
		return false;
	}

	if (!isDelcaredScopeVariable(interpreter->scope, lhs)) {
		printf("Undeclared variable \"");
		printLiteral(lhs);
		printf("\"\n");
		return false;
	}

	if (!setScopeVariable(interpreter->scope, lhs, rhs, true)) {
		printf("Incorrect type assigned to variable \"");
		printLiteral(lhs);
		printf("\"\n");
		return false;
	}

	return true;
}

static bool execValCast(Interpreter* interpreter) {
	Literal value = popLiteralArray(&interpreter->stack);
	Literal type = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &value)) {
		return false;
	}

	Literal result = TO_NULL_LITERAL;

	if (IS_NULL(value)) {
		printf("Can't cast a null value\n");
		return false;
	}

	//cast the rhs to the type represented by lhs
	switch(AS_TYPE(type).typeOf) {
		case LITERAL_BOOLEAN:
			result = TO_BOOLEAN_LITERAL(IS_TRUTHY(value));
		break;

		case LITERAL_INTEGER:
			if (IS_BOOLEAN(value)) {
				result = TO_INTEGER_LITERAL(AS_BOOLEAN(value) ? 1 : 0);
			}

			if (IS_FLOAT(value)) {
				result = TO_INTEGER_LITERAL(AS_FLOAT(value));
			}

			if (IS_STRING(value)) {
				int val = 0;
				sscanf(AS_STRING(value), "%d", &val);
				result = TO_INTEGER_LITERAL(val);
			}
		break;

		case LITERAL_FLOAT:
			if (IS_BOOLEAN(value)) {
				result = TO_FLOAT_LITERAL(AS_BOOLEAN(value) ? 1 : 0);
			}

			if (IS_INTEGER(value)) {
				result = TO_FLOAT_LITERAL(AS_INTEGER(value));
			}

			if (IS_STRING(value)) {
				float val = 0;
				sscanf(AS_STRING(value), "%f", &val);
				result = TO_FLOAT_LITERAL(val);
			}
		break;

		case LITERAL_STRING:
			if (IS_BOOLEAN(value)) {
				result = TO_STRING_LITERAL(AS_BOOLEAN(value) ? "true" : "false");
			}

			if (IS_INTEGER(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%d", AS_INTEGER(value));
				result = TO_STRING_LITERAL(buffer);
			}

			if (IS_FLOAT(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%g", AS_FLOAT(value));
				result = TO_STRING_LITERAL(buffer);
			}
		break;

		default:
			printf("Unknown cast type found %d, terminating\n", AS_TYPE(type).typeOf);
			return false;
	}

	//leave the new value on the stack
	pushLiteralArray(&interpreter->stack, result);
	return true;
}

static bool execCompareEqual(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	parseIdentifierToValue(interpreter, &rhs);
	parseIdentifierToValue(interpreter, &lhs);

	bool result = literalsAreEqual(lhs, rhs);

	if (invert) {
		result = !result;
	}

	pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(result));

	return true;
}

static bool execCompareLess(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	parseIdentifierToValue(interpreter, &rhs);
	parseIdentifierToValue(interpreter, &lhs);

	//not a number, return falure
	if (!(IS_INTEGER(lhs) || IS_FLOAT(lhs))) {
		printf("Incorrect type in comparison, value \"");
		printLiteral(lhs);
		printf("\"\n");
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		printf("Incorrect type in comparison, value \"");
		printLiteral(rhs);
		printf("\"\n");
		return false;
	}

	//convert to floats - easier
	if (IS_INTEGER(lhs)) {
		lhs = TO_FLOAT_LITERAL(AS_INTEGER(lhs));
	}

	if (IS_INTEGER(rhs)) {
		rhs = TO_FLOAT_LITERAL(AS_INTEGER(rhs));
	}

	bool result;

	if (!invert) {
		result = (AS_FLOAT(lhs) < AS_FLOAT(rhs));
	}
	else {
		result = (AS_FLOAT(lhs) > AS_FLOAT(rhs));
	}

	pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(result));

	return true;
}

static bool execCompareLessEqual(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	parseIdentifierToValue(interpreter, &rhs);
	parseIdentifierToValue(interpreter, &lhs);

	//not a number, return falure
	if (!(IS_INTEGER(lhs) || IS_FLOAT(lhs))) {
		printf("Incorrect type in comparison, value \"");
		printLiteral(lhs);
		printf("\"\n");
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		printf("Incorrect type in comparison, value \"");
		printLiteral(rhs);
		printf("\"\n");
		return false;
	}

	//convert to floats - easier
	if (IS_INTEGER(lhs)) {
		lhs = TO_FLOAT_LITERAL(AS_INTEGER(lhs));
	}

	if (IS_INTEGER(rhs)) {
		rhs = TO_FLOAT_LITERAL(AS_INTEGER(rhs));
	}

	bool result;

	if (!invert) {
		result = (AS_FLOAT(lhs) < AS_FLOAT(rhs)) || literalsAreEqual(lhs, rhs);
	}
	else {
		result = (AS_FLOAT(lhs) > AS_FLOAT(rhs)) || literalsAreEqual(lhs, rhs);
	}

	pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(result));

	return true;
}

static bool execJump(Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		printf("Jump out of range\n");
		return false;
	}

	//actually jump
	interpreter->count = target + interpreter->codeStart;

	return true;
}

static bool execFalseJump(Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		printf("Jump out of range (false jump)\n");
		return false;
	}

	//actually jump
	Literal lit = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	if (IS_NULL(lit)) {
		printf("Null detected in comparison\n");
		return false;
	}

	if (!IS_TRUTHY(lit)) {
		interpreter->count = target + interpreter->codeStart;
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

			case OP_LITERAL_RAW:
				if (!rawLiteral(interpreter)) {
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

			//TODO: custom type declarations

			case OP_VAR_DECL:
			case OP_VAR_DECL_LONG:
				if (!execVarDecl(interpreter, opcode == OP_VAR_DECL_LONG)) {
					return;
				}
			break;

			case OP_VAR_ASSIGN:
				if (!execVarAssign(interpreter)) {
					return;
				}
			break;

			case OP_TYPE_CAST:
				if (!execValCast(interpreter)) {
					return;
				}
			break;

			case OP_COMPARE_EQUAL:
				if (!execCompareEqual(interpreter, false)) {
					return;
				}
			break;

			case OP_COMPARE_NOT_EQUAL:
				if (!execCompareEqual(interpreter, true)) {
					return;
				}
			break;

			case OP_COMPARE_LESS:
				if (!execCompareLess(interpreter, false)) {
					return;
				}
			break;

			case OP_COMPARE_LESS_EQUAL:
				if (!execCompareLessEqual(interpreter, false)) {
					return;
				}
			break;

			case OP_COMPARE_GREATER:
				if (!execCompareLess(interpreter, true)) {
					return;
				}
			break;

			case OP_COMPARE_GREATER_EQUAL:
				if (!execCompareLessEqual(interpreter, true)) {
					return;
				}
			break;

			case OP_INVERT:
				if (!execInvert(interpreter)) {
					return;
				}
			break;

			case OP_JUMP:
				if (!execJump(interpreter)) {
					return;
				}
			break;

			case OP_IF_FALSE_JUMP:
				if (!execFalseJump(interpreter)) {
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

void runInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length) {
	//prep the bytecode
	interpreter->bytecode = bytecode;
	interpreter->length = length;
	interpreter->count = 0;

	if (!interpreter->bytecode) {
		printf(ERROR "Error: No valid bytecode given\n" RESET);
		return;
	}

	//prep the literal cache
	if (interpreter->literalCache.count > 0) {
		freeLiteralArray(&interpreter->literalCache); //automatically inits
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
					printf("(identifier %s (hash: %x))\n", AS_IDENTIFIER(identifier), identifier.as.identifier.hash);
				}
			}
			break;

			case LITERAL_TYPE: {
				//what the literal is
				LiteralType literalType = (LiteralType)readByte(interpreter->bytecode, &interpreter->count);
				unsigned char constant = readByte(interpreter->bytecode, &interpreter->count);

				Literal typeLiteral = TO_TYPE_LITERAL(literalType, constant);

				//save the type
				pushLiteralArray(&interpreter->literalCache, typeLiteral);

				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}
			}
			break;

			case LITERAL_TYPE_INTERMEDIATE: {
				//what the literal represents
				LiteralType literalType = (LiteralType)readByte(interpreter->bytecode, &interpreter->count);
				unsigned char constant = readByte(interpreter->bytecode, &interpreter->count);

				Literal typeLiteral = TO_TYPE_LITERAL(literalType, constant);

				//if it's an array type
				if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY) {
					unsigned short vt = readShort(interpreter->bytecode, &interpreter->count);

					TYPE_PUSH_SUBTYPE(&typeLiteral, interpreter->literalCache.literals[vt]);
				}

				if (AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY) {
					unsigned short kt = readShort(interpreter->bytecode, &interpreter->count);
					unsigned short vt = readShort(interpreter->bytecode, &interpreter->count);

					TYPE_PUSH_SUBTYPE(&typeLiteral, interpreter->literalCache.literals[kt]);
					TYPE_PUSH_SUBTYPE(&typeLiteral, interpreter->literalCache.literals[vt]);
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

	//set the starting point for the interpreter
	interpreter->codeStart = interpreter->count;

	//code section
	if (command.verbose) {
		printf(NOTICE "executing bytecode\n" RESET);
	}

	execInterpreter(interpreter);
}
