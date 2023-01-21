#include "interpreter.h"
#include "console_colors.h"

#include "toy_common.h"
#include "memory.h"
#include "keyword_types.h"
#include "opcodes.h"

#include "builtin.h"

#include <stdio.h>
#include <string.h>

static void printWrapper(const char* output) {
	printf("%s", output);
	printf("\n"); //default new line
}

static void assertWrapper(const char* output) {
	fprintf(stderr, ERROR "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n" RESET); //default new line
}

static void errorWrapper(const char* output) {
	fprintf(stderr, ERROR "%s" RESET, output); //no newline
}

bool injectNativeFn(Interpreter* interpreter, char* name, NativeFn func) {
	//reject reserved words
	if (findTypeByKeyword(name) != TOKEN_EOF) {
		interpreter->errorOutput("Can't override an existing keyword\n");
		return false;
	}

	int identifierLength = strlen(name);
	Literal identifier = TO_IDENTIFIER_LITERAL(createRefStringLength(name, identifierLength));

	//make sure the name isn't taken
	if (existsLiteralDictionary(&interpreter->scope->variables, identifier)) {
		interpreter->errorOutput("Can't override an existing variable\n");
		return false;
	}

	Literal fn = TO_FUNCTION_LITERAL((void*)func, 0);
	fn.type = LITERAL_FUNCTION_NATIVE;

	Literal type = TO_TYPE_LITERAL(fn.type, true);

	setLiteralDictionary(&interpreter->scope->variables, identifier, fn);
	setLiteralDictionary(&interpreter->scope->types, identifier, type);

	freeLiteral(identifier);
	freeLiteral(type);

	return true;
}

bool injectNativeHook(Interpreter* interpreter, char* name, HookFn hook) {
	//reject reserved words
	if (findTypeByKeyword(name) != TOKEN_EOF) {
		interpreter->errorOutput("Can't inject a hook on an existing keyword\n");
		return false;
	}

	int identifierLength = strlen(name);
	Literal identifier = TO_IDENTIFIER_LITERAL(createRefStringLength(name, identifierLength));

	//make sure the name isn't taken
	if (existsLiteralDictionary(interpreter->hooks, identifier)) {
		interpreter->errorOutput("Can't override an existing hook\n");
		return false;
	}

	Literal fn = TO_FUNCTION_LITERAL((void*)hook, 0);
	fn.type = LITERAL_FUNCTION_NATIVE;

	setLiteralDictionary(interpreter->hooks, identifier, fn);

	freeLiteral(identifier);

	return true;
}

void parseCompoundToPureValues(Interpreter* interpreter, Literal* literalPtr) {
	if (IS_IDENTIFIER(*literalPtr)) {
		parseIdentifierToValue(interpreter, literalPtr);
	}

	//parse out an array
	if (IS_ARRAY(*literalPtr)) {
		for (int i = 0; i < AS_ARRAY(*literalPtr)->count; i++) {
			Literal index = TO_INTEGER_LITERAL(i);
			Literal entry = getLiteralArray(AS_ARRAY(*literalPtr), index);

			if (IS_IDENTIFIER( entry )) {
				Literal idn = entry;
				parseCompoundToPureValues(interpreter, &entry);

				setLiteralArray(AS_ARRAY(*literalPtr), index, entry);

				freeLiteral(idn);
			}

			freeLiteral(index);
			freeLiteral(entry);
		}
	}

	//parse out a dictionary
	if (IS_DICTIONARY(*literalPtr)) {
		LiteralDictionary* ret = ALLOCATE(LiteralDictionary, 1);
		initLiteralDictionary(ret);

		for (int i = 0; i < AS_DICTIONARY(*literalPtr)->capacity; i++) {
			if ( IS_NULL(AS_DICTIONARY(*literalPtr)->entries[i].key) ) {
				continue;
			}

			Literal key = TO_NULL_LITERAL;
			Literal value = TO_NULL_LITERAL;

			key = copyLiteral(AS_DICTIONARY(*literalPtr)->entries[i].key);
			value = copyLiteral(AS_DICTIONARY(*literalPtr)->entries[i].value);

			//
			if (IS_IDENTIFIER( key ) || IS_IDENTIFIER(value)) {
				parseCompoundToPureValues(interpreter, &key);
				parseCompoundToPureValues(interpreter, &value);
			}

			setLiteralDictionary(ret, key, value);

			//
			freeLiteral(key);
			freeLiteral(value);
		}

		freeLiteral(*literalPtr);
		*literalPtr = TO_DICTIONARY_LITERAL(ret);
	}
}

bool parseIdentifierToValue(Interpreter* interpreter, Literal* literalPtr) {
	//this converts identifiers to values
	if (IS_IDENTIFIER(*literalPtr)) {
		if (!getScopeVariable(interpreter->scope, *literalPtr, literalPtr)) {
			interpreter->errorOutput("Undeclared variable ");
			printLiteralCustom(*literalPtr, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return false;
		}
	}

	if (IS_ARRAY(*literalPtr) || IS_DICTIONARY(*literalPtr)) {
		parseCompoundToPureValues(interpreter, literalPtr);
	}

	return true;
}

//utilities for the host program
void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput) {
	interpreter->printOutput = printOutput;
}

void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput) {
	interpreter->assertOutput = assertOutput;
}

void setInterpreterError(Interpreter* interpreter, PrintFn errorOutput) {
	interpreter->errorOutput = errorOutput;
}

//utils
static unsigned char readByte(unsigned char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	*count += 1;
	return ret;
}

static unsigned short readShort(unsigned char* tb, int* count) {
	unsigned short ret = 0;
	memcpy(&ret, tb + *count, 2);
	*count += 2;
	return ret;
}

static int readInt(unsigned char* tb, int* count) {
	int ret = 0;
	memcpy(&ret, tb + *count, 4);
	*count += 4;
	return ret;
}

static float readFloat(unsigned char* tb, int* count) {
	float ret = 0;
	memcpy(&ret, tb + *count, 4);
	*count += 4;
	return ret;
}

static char* readString(unsigned char* tb, int* count) {
	unsigned char* ret = tb + *count;
	*count += strlen((char*)ret) + 1; //+1 for null character
	return (char*)ret;
}

static void consumeByte(Interpreter* interpreter, unsigned char byte, unsigned char* tb, int* count) {
	if (byte != tb[*count]) {
		char buffer[512];
		snprintf(buffer, 512, "[internal] Failed to consume the correct byte (expected %u, found %u)\n", byte, tb[*count]);
		interpreter->errorOutput(buffer);
	}
	*count += 1;
}

static void consumeShort(Interpreter* interpreter, unsigned short bytes, unsigned char* tb, int* count) {
	if (bytes != *(unsigned short*)(tb + *count)) {
		char buffer[512];
		snprintf(buffer, 512, "[internal] Failed to consume the correct bytes (expected %u, found %u)\n", bytes, *(unsigned short*)(tb + *count));
		interpreter->errorOutput(buffer);
	}
	*count += 2;
}

//each available statement
static bool execAssert(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);
	parseIdentifierToValue(interpreter, &lhs);

	if (!IS_STRING(rhs)) {
		interpreter->errorOutput("The assert keyword needs a string as the second argument, received: ");
		printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		return false;
	}

	if (IS_NULL(lhs) || !IS_TRUTHY(lhs)) {
		(*interpreter->assertOutput)(toCString(AS_STRING(rhs)));
		freeLiteral(rhs);
		interpreter->panic = true;
		return false;
	}

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execPrint(Interpreter* interpreter) {
	//print what is on top of the stack, then pop it
	Literal lit = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(lit)) {
		Literal idn = lit;
		if (!parseIdentifierToValue(interpreter, &lit)) {
			return false;
		}
		freeLiteral(idn);
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

	if (IS_IDENTIFIER(lit)) {
		Literal idn = lit;
		if (!parseIdentifierToValue(interpreter, &lit)) {
			return false;
		}
		freeLiteral(idn);
	}

	pushLiteralArray(&interpreter->stack, lit);

	freeLiteral(lit);

	return true;
}

static bool execNegate(Interpreter* interpreter) {
	//negate the top literal on the stack (numbers only)
	Literal lit = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(lit)) {
		Literal idn = lit;
		if (!parseIdentifierToValue(interpreter, &lit)) {
			return false;
		}
		freeLiteral(idn);
	}

	if (IS_INTEGER(lit)) {
		lit = TO_INTEGER_LITERAL(-AS_INTEGER(lit));
	}
	else if (IS_FLOAT(lit)) {
		lit = TO_FLOAT_LITERAL(-AS_FLOAT(lit));
	}
	else {
		interpreter->errorOutput("Can't negate that literal: ");
		printLiteralCustom(lit, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		freeLiteral(lit);

		return false;
	}

	pushLiteralArray(&interpreter->stack, lit);

	freeLiteral(lit);

	return true;
}

static bool execInvert(Interpreter* interpreter) {
	//negate the top literal on the stack (booleans only)
	Literal lit = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(lit)) {
		Literal idn = lit;
		if (!parseIdentifierToValue(interpreter, &lit)) {
			return false;
		}
		freeLiteral(idn);
	}

	if (IS_BOOLEAN(lit)) {
		lit = TO_BOOLEAN_LITERAL(!AS_BOOLEAN(lit));
	}
	else {
		interpreter->errorOutput("Can't invert that literal: ");
		printLiteralCustom(lit, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		freeLiteral(lit);

		return false;
	}

	pushLiteralArray(&interpreter->stack, lit);

	freeLiteral(lit);

	return true;
}

static bool execArithmetic(Interpreter* interpreter, Opcode opcode) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	//special case for string concatenation ONLY
	if (IS_STRING(lhs) && IS_STRING(rhs)) {
		//check for overflow
		int totalLength = AS_STRING(lhs)->length + AS_STRING(rhs)->length;
		if (totalLength > MAX_STRING_LENGTH) {
			interpreter->errorOutput("Can't concatenate these strings (result is too long)\n");
			return false;
		}

		//concat the strings
		char buffer[MAX_STRING_LENGTH];
		snprintf(buffer, MAX_STRING_LENGTH, "%s%s", toCString(AS_STRING(lhs)), toCString(AS_STRING(rhs)));
		Literal literal = TO_STRING_LITERAL(createRefStringLength(buffer, totalLength));
		pushLiteralArray(&interpreter->stack, literal);

		//cleanup
		freeLiteral(literal);
		freeLiteral(lhs);
		freeLiteral(rhs);

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
			case OP_VAR_ADDITION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) + AS_INTEGER(rhs) ));
				return true;

			case OP_SUBTRACTION:
			case OP_VAR_SUBTRACTION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) - AS_INTEGER(rhs) ));
				return true;

			case OP_MULTIPLICATION:
			case OP_VAR_MULTIPLICATION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) * AS_INTEGER(rhs) ));
				return true;

			case OP_DIVISION:
			case OP_VAR_DIVISION_ASSIGN:
				if (AS_INTEGER(rhs) == 0) {
					interpreter->errorOutput("Can't divide by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) / AS_INTEGER(rhs) ));
				return true;

			case OP_MODULO:
			case OP_VAR_MODULO_ASSIGN:
				if (AS_INTEGER(rhs) == 0) {
					interpreter->errorOutput("Can't modulo by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) % AS_INTEGER(rhs) ));
				return true;

			default:
				interpreter->errorOutput("[internal] bad opcode argument passed to execArithmetic()");
				return false;
		}
	}

	//catch bad modulo
	if (opcode == OP_MODULO || opcode == OP_VAR_MODULO_ASSIGN) {
		interpreter->errorOutput("Bad arithmetic argument (modulo on floats not allowed)\n");
		return false;
	}

	if(IS_FLOAT(lhs) && IS_FLOAT(rhs)) {
		switch(opcode) {
			case OP_ADDITION:
			case OP_VAR_ADDITION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) + AS_FLOAT(rhs) ));
				return true;

			case OP_SUBTRACTION:
			case OP_VAR_SUBTRACTION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) - AS_FLOAT(rhs) ));
				return true;

			case OP_MULTIPLICATION:
			case OP_VAR_MULTIPLICATION_ASSIGN:
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) * AS_FLOAT(rhs) ));
				return true;

			case OP_DIVISION:
			case OP_VAR_DIVISION_ASSIGN:
				if (AS_FLOAT(rhs) == 0) {
					interpreter->errorOutput("Can't divide by zero (error found in interpreter)");
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) / AS_FLOAT(rhs) ));
				return true;

			default:
				interpreter->errorOutput("[internal] bad opcode argument passed to execArithmetic()");
				return false;
		}
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	freeLiteral(lhs);
	freeLiteral(rhs);

	return false;
}

static Literal parseTypeToValue(Interpreter* interpreter, Literal type) {
	//if an identifier is embedded in the type, figure out what it iss
	if (IS_IDENTIFIER(type)) {
		Literal idn = type;
		parseIdentifierToValue(interpreter, &type);
		freeLiteral(idn);
	}

	//if this is an array or dictionary, continue to the subtypes
	if (IS_TYPE(type) && (AS_TYPE(type).typeOf == LITERAL_ARRAY || AS_TYPE(type).typeOf == LITERAL_DICTIONARY)) {
		for (int i = 0; i < AS_TYPE(type).count; i++) {
			((Literal*)(AS_TYPE(type).subtypes))[i] = parseTypeToValue(interpreter, ((Literal*)(AS_TYPE(type).subtypes))[i]);
		}
	}

	//BUGFIX: make sure it actually is a type
	if (!IS_TYPE(type)) {
		interpreter->errorOutput("Bad type encountered: ");
		printLiteralCustom(type, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		//TODO: would be better to return an int here...
	}

	return type;
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
	Literal type = copyLiteral(interpreter->literalCache.literals[typeIndex]);

	if (IS_IDENTIFIER(type)) {
		Literal orig = type;
		parseIdentifierToValue(interpreter, &type);
		freeLiteral(orig);
	}

	//BUGFIX: because identifiers are getting embedded in type definitions
	type = parseTypeToValue(interpreter, type);

	if (!declareScopeVariable(interpreter->scope, identifier, type)) {
		interpreter->errorOutput("Can't redefine the variable \"");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	Literal val = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(val)) {
		Literal idn = val;
		parseIdentifierToValue(interpreter, &val);
		freeLiteral(idn);
	}

	if (IS_ARRAY(val) || IS_DICTIONARY(val)) {
		parseCompoundToPureValues(interpreter, &val);
	}

	//TODO: could restrict opaque data to only opaque variables

	//BUGFIX: allow easy coercion on decl
	if (AS_TYPE(type).typeOf == LITERAL_FLOAT && IS_INTEGER(val)) {
		val = TO_FLOAT_LITERAL(AS_INTEGER(val));
	}

	if (!IS_NULL(val) && !setScopeVariable(interpreter->scope, identifier, val, false)) {
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(type);
		freeLiteral(val);

		return false;
	}

	freeLiteral(val);
	freeLiteral(type);

	return true;
}

static bool execFnDecl(Interpreter* interpreter, bool lng) {
	//read the index in the cache
	int identifierIndex = 0;
	int functionIndex = 0;

	if (lng) {
		identifierIndex = (int)readShort(interpreter->bytecode, &interpreter->count);
		functionIndex = (int)readShort(interpreter->bytecode, &interpreter->count);
	}
	else {
		identifierIndex = (int)readByte(interpreter->bytecode, &interpreter->count);
		functionIndex = (int)readByte(interpreter->bytecode, &interpreter->count);
	}

	Literal identifier = interpreter->literalCache.literals[identifierIndex];
	Literal function = interpreter->literalCache.literals[functionIndex];

	AS_FUNCTION(function).scope = pushScope(interpreter->scope); //hacked in (needed for closure persistance)

	Literal type = TO_TYPE_LITERAL(LITERAL_FUNCTION, true);

	if (!declareScopeVariable(interpreter->scope, identifier, type)) {
		interpreter->errorOutput("Can't redefine the function \"");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	if (!setScopeVariable(interpreter->scope, identifier, function, false)) { //scope gets copied here
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	popScope(AS_FUNCTION(function).scope); //hacked out
	AS_FUNCTION(function).scope = NULL;

	freeLiteral(type);

	return true;
}

static bool execVarAssign(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_ARRAY(rhs) || IS_DICTIONARY(rhs)) {
		parseCompoundToPureValues(interpreter, &rhs);
	}

	if (!IS_IDENTIFIER(lhs)) {
		interpreter->errorOutput("Can't assign to a non-variable \"");
		printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	if (!isDelcaredScopeVariable(interpreter->scope, lhs)) {
		interpreter->errorOutput("Undeclared variable \"");
		printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(lhs);
		freeLiteral(rhs);
		return false;
	}

	//BUGFIX: allow easy coercion on assign
	Literal type = getScopeType(interpreter->scope, lhs);
	if (AS_TYPE(type).typeOf == LITERAL_FLOAT && IS_INTEGER(rhs)) {
		rhs = TO_FLOAT_LITERAL(AS_INTEGER(rhs));
	}

	if (!setScopeVariable(interpreter->scope, lhs, rhs, true)) {
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(lhs);
		freeLiteral(rhs);
		freeLiteral(type);
		return false;
	}

	freeLiteral(lhs);
	freeLiteral(rhs);
	freeLiteral(type);

	return true;
}

static bool execVarArithmeticAssign(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	//duplicate the name
	pushLiteralArray(&interpreter->stack, lhs);
	pushLiteralArray(&interpreter->stack, lhs);
	pushLiteralArray(&interpreter->stack, rhs);

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execValCast(Interpreter* interpreter) {
	Literal value = popLiteralArray(&interpreter->stack);
	Literal type = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(value)) {
		Literal idn = value;
		if (!parseIdentifierToValue(interpreter, &value)) {
			return false;
		}
		freeLiteral(idn);
	}

	Literal result = TO_NULL_LITERAL;

	if (IS_NULL(value)) {
		interpreter->errorOutput("Can't cast a null value\n");

		freeLiteral(value);
		freeLiteral(type);

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

			if (IS_INTEGER(value)) {
				result = copyLiteral(value);
			}

			if (IS_FLOAT(value)) {
				result = TO_INTEGER_LITERAL(AS_FLOAT(value));
			}

			if (IS_STRING(value)) {
				int val = 0;
				sscanf(toCString(AS_STRING(value)), "%d", &val);
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

			if (IS_FLOAT(value)) {
				result = copyLiteral(value);
			}

			if (IS_STRING(value)) {
				float val = 0;
				sscanf(toCString(AS_STRING(value)), "%f", &val);
				result = TO_FLOAT_LITERAL(val);
			}
		break;

		case LITERAL_STRING:
			if (IS_BOOLEAN(value)) {
				char* str = AS_BOOLEAN(value) ? "true" : "false";

				int length = strlen(str);
				result = TO_STRING_LITERAL(createRefStringLength(str, length)); //TODO: static reference optimisation?
			}

			if (IS_INTEGER(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%d", AS_INTEGER(value));
				int length = strlen(buffer);
				result = TO_STRING_LITERAL(createRefStringLength(buffer, length));
			}

			if (IS_FLOAT(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%g", AS_FLOAT(value));
				int length = strlen(buffer);
				result = TO_STRING_LITERAL(createRefStringLength(buffer, length));
			}

			if (IS_STRING(value)) {
				result = copyLiteral(value);
			}
		break;

		default:
			interpreter->errorOutput("Unknown cast type found: ");
			printLiteralCustom(type, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return false;
	}

	//leave the new value on the stack
	pushLiteralArray(&interpreter->stack, result);

	freeLiteral(result);
	freeLiteral(value);
	freeLiteral(type);

	return true;
}

static bool execTypeOf(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal type = TO_NULL_LITERAL;

	if (IS_IDENTIFIER(rhs)) {
		type = getScopeType(interpreter->scope, rhs);
	}
	else {
		type = TO_TYPE_LITERAL(rhs.type, false);
	}

	pushLiteralArray(&interpreter->stack, type);

	freeLiteral(rhs);
	freeLiteral(type);

	return true;
}

static bool execCompareEqual(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	bool result = literalsAreEqual(lhs, rhs);

	if (invert) {
		result = !result;
	}

	pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(result));

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execCompareLess(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	//not a number, return falure
	if (!(IS_INTEGER(lhs) || IS_FLOAT(lhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(lhs);
		freeLiteral(rhs);
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		freeLiteral(lhs);
		freeLiteral(rhs);
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

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execCompareLessEqual(Interpreter* interpreter, bool invert) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	//not a number, return falure
	if (!(IS_INTEGER(lhs) || IS_FLOAT(lhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(lhs);
		freeLiteral(rhs);
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(lhs);
		freeLiteral(rhs);
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

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execAnd(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	if (IS_TRUTHY(lhs) && IS_TRUTHY(rhs)) {
		pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(true));
	}
	else {
		pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(false));
	}

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execOr(Interpreter* interpreter) {
	Literal rhs = popLiteralArray(&interpreter->stack);
	Literal lhs = popLiteralArray(&interpreter->stack);

	if (IS_IDENTIFIER(rhs)) {
		Literal idn = rhs;
		parseIdentifierToValue(interpreter, &rhs);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(lhs)) {
		Literal idn = lhs;
		parseIdentifierToValue(interpreter, &lhs);
		freeLiteral(idn);
	}

	if (IS_TRUTHY(lhs) || IS_TRUTHY(rhs)) {
		pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(true));
	}
	else {
		pushLiteralArray(&interpreter->stack, TO_BOOLEAN_LITERAL(false));
	}

	freeLiteral(lhs);
	freeLiteral(rhs);

	return true;
}

static bool execJump(Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		interpreter->errorOutput("[internal] Jump out of range\n");
		return false;
	}

	//actually jump
	interpreter->count = target + interpreter->codeStart;

	return true;
}

static bool execFalseJump(Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		interpreter->errorOutput("[internal] Jump out of range (false jump)\n");
		return false;
	}

	//actually jump
	Literal lit = popLiteralArray(&interpreter->stack);

	bool freeLit = false;
	if (IS_IDENTIFIER(lit)) {
		Literal idn = lit;
		parseIdentifierToValue(interpreter, &lit);
		freeLiteral(idn);
		freeLit = true;
	}

	if (IS_NULL(lit)) {
		interpreter->errorOutput("Null detected in comparison\n");
		return false;
	}

	if (!IS_TRUTHY(lit)) {
		interpreter->count = target + interpreter->codeStart;
	}

	if (freeLit) {
		freeLiteral(lit);
	}

	return true;
}

//forward declare
static void execInterpreter(Interpreter*);
static void readInterpreterSections(Interpreter* interpreter);

static bool execFnCall(Interpreter* interpreter, bool looseFirstArgument) {
	//BUGFIX: depth check - don't drown!
	if (interpreter->depth >= 200) {
		interpreter->errorOutput("Infinite recursion detected - panicking\n");
		interpreter->panic = true;
		return false;
	}

	LiteralArray arguments;
	initLiteralArray(&arguments);

	Literal stackSize = popLiteralArray(&interpreter->stack);

	//unpack the stack of arguments
	for (int i = 0; i < AS_INTEGER(stackSize) - 1; i++) {
		Literal lit = popLiteralArray(&interpreter->stack);
		pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	//collect one more argument
	if (!looseFirstArgument && AS_INTEGER(stackSize) > 0) {
		Literal lit = popLiteralArray(&interpreter->stack);
		pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	Literal identifier = popLiteralArray(&interpreter->stack);

	//collect one more argument
	if (looseFirstArgument) {
		Literal lit = popLiteralArray(&interpreter->stack);
		pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	//let's screw with the fn name, too
	if (looseFirstArgument) {
		int length = AS_IDENTIFIER(identifier)->length + 1;
		char buffer[MAX_STRING_LENGTH];
		snprintf(buffer, MAX_STRING_LENGTH, "_%s", toCString(AS_IDENTIFIER(identifier))); //prepend an underscore

		freeLiteral(identifier);
		identifier = TO_IDENTIFIER_LITERAL(createRefStringLength(buffer, length));
	}

	Literal func = identifier;

	if (!parseIdentifierToValue(interpreter, &func)) {
		freeLiteralArray(&arguments);
		freeLiteral(identifier);
		return false;
	}

	//check for side-loaded native functions
	if (IS_FUNCTION_NATIVE(func)) {
		//reverse the order to the correct order
		LiteralArray correct;
		initLiteralArray(&correct);

		while(arguments.count) {
			Literal lit =  popLiteralArray(&arguments);
			pushLiteralArray(&correct, lit);
			freeLiteral(lit);
		}

		freeLiteralArray(&arguments);

		//call the native function
		((NativeFn) AS_FUNCTION(func).bytecode )(interpreter, &correct);

		freeLiteralArray(&correct);
		freeLiteral(identifier);
		return true;
	}

	if (!IS_FUNCTION(func)) {
		interpreter->errorOutput("Function not found: ");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		freeLiteral(identifier);
		freeLiteralArray(&arguments);
		return false;
	}

	bool ret = callLiteralFn(interpreter, func, &arguments, &interpreter->stack);

	if (!ret) {
		interpreter->errorOutput("Error encountered in function \"");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
	}

	freeLiteralArray(&arguments);
	freeLiteral(func);
	freeLiteral(identifier);

	return ret;
}

bool callLiteralFn(Interpreter* interpreter, Literal func, LiteralArray* arguments, LiteralArray* returns) {
	if (!IS_FUNCTION(func)) {
		interpreter->errorOutput("Function required in callLiteralFn()\n");
		return false;
	}

	//set up a new interpreter
	Interpreter inner;

	//init the inner interpreter manually
	initLiteralArray(&inner.literalCache);
	inner.scope = pushScope(func.as.function.scope);
	inner.bytecode = AS_FUNCTION(func).bytecode;
	inner.length = AS_FUNCTION(func).length;
	inner.count = 0;
	inner.codeStart = -1;
	inner.depth = interpreter->depth + 1;
	inner.panic = false;
	initLiteralArray(&inner.stack);
	inner.hooks = interpreter->hooks;
	setInterpreterPrint(&inner, interpreter->printOutput);
	setInterpreterAssert(&inner, interpreter->assertOutput);
	setInterpreterError(&inner, interpreter->errorOutput);

	//prep the sections
	readInterpreterSections(&inner);

	//prep the arguments
	LiteralArray* paramArray = AS_ARRAY(inner.literalCache.literals[ readShort(inner.bytecode, &inner.count) ]);
	LiteralArray* returnArray = AS_ARRAY(inner.literalCache.literals[ readShort(inner.bytecode, &inner.count) ]);

	//get the rest param, if it exists
	Literal restParam = TO_NULL_LITERAL;
	if (paramArray->count >= 2 && AS_TYPE(paramArray->literals[ paramArray->count -1 ]).typeOf == LITERAL_FUNCTION_ARG_REST) {
		restParam = paramArray->literals[ paramArray->count -2 ];
	}

	//check the param total is correct
	if ((IS_NULL(restParam) && paramArray->count != arguments->count * 2) || (!IS_NULL(restParam) && paramArray->count -2 > arguments->count * 2)) {
		interpreter->errorOutput("Incorrect number of arguments passed to a function\n");

		//free, and skip out
		popScope(inner.scope);

		freeLiteralArray(&inner.stack);
		freeLiteralArray(&inner.literalCache);

		return false;
	}

	//contents is the indexes of identifier & type
	for (int i = 0; i < paramArray->count - (IS_NULL(restParam) ? 0 : 2); i += 2) { //don't count the rest parameter, if present
		//declare and define each entry in the scope
		if (!declareScopeVariable(inner.scope, paramArray->literals[i], paramArray->literals[i + 1])) {
			interpreter->errorOutput("[internal] Could not re-declare parameter\n");

			//free, and skip out
			popScope(inner.scope);

			freeLiteralArray(&inner.stack);
			freeLiteralArray(&inner.literalCache);

			return false;
		}

		Literal arg = popLiteralArray(arguments);

		if (IS_IDENTIFIER(arg)) {
			Literal idn = arg;
			parseIdentifierToValue(interpreter, &arg);
			freeLiteral(idn);
		}

		if (!setScopeVariable(inner.scope, paramArray->literals[i], arg, false)) {
			interpreter->errorOutput("[internal] Could not define parameter (bad type?)\n");

			//free, and skip out
			freeLiteral(arg);
			popScope(inner.scope);

			freeLiteralArray(&inner.stack);
			freeLiteralArray(&inner.literalCache);

			return false;
		}
		freeLiteral(arg);
	}

	//if using rest, pack the optional extra arguments into the rest parameter (array)
	if (!IS_NULL(restParam)) {
		LiteralArray rest;
		initLiteralArray(&rest);

		while (arguments->count > 0) {
			Literal lit = popLiteralArray(arguments);
			pushLiteralArray(&rest, lit);
			freeLiteral(lit);
		}

		Literal restType = TO_TYPE_LITERAL(LITERAL_ARRAY, true);
		Literal any = TO_TYPE_LITERAL(LITERAL_ANY, false);
		TYPE_PUSH_SUBTYPE(&restType, any);

		//declare & define the rest parameter
		if (!declareScopeVariable(inner.scope, restParam, restType)) {
			interpreter->errorOutput("[internal] Could not declare rest parameter\n");

			//free, and skip out
			freeLiteral(restType);
			freeLiteralArray(&rest);
			popScope(inner.scope);

			freeLiteralArray(&inner.stack);
			freeLiteralArray(&inner.literalCache);

			return false;
		}

		Literal lit = TO_ARRAY_LITERAL(&rest);
		if (!setScopeVariable(inner.scope, restParam, lit, false)) {
			interpreter->errorOutput("[internal] Could not define rest parameter\n");

			//free, and skip out
			freeLiteral(restType);
			freeLiteral(lit);
			popScope(inner.scope);

			freeLiteralArray(&inner.stack);
			freeLiteralArray(&inner.literalCache);

			return false;
		}

		freeLiteral(restType);
		freeLiteralArray(&rest);
	}

	//execute the interpreter
	execInterpreter(&inner);

	//adopt the panic state
	interpreter->panic = inner.panic;

	//accept the stack as the results
	LiteralArray returnsFromInner;
	initLiteralArray(&returnsFromInner);

	//unpack the results
	for (int i = 0; i < (returnArray->count || 1); i++) {
		Literal lit = popLiteralArray(&inner.stack);
		pushLiteralArray(&returnsFromInner, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	bool returnValue = true;

	//TODO: remove this when multiple assignment is enabled - note the BUGFIX that balances the stack
	if (returnsFromInner.count > 1) {
		interpreter->errorOutput("Too many values returned (multiple returns not yet supported)\n");

		returnValue = false;
	}

	for (int i = 0; i < returnsFromInner.count && returnValue; i++) {
		Literal ret = popLiteralArray(&returnsFromInner);

		//check the return types
		if (returnArray->count > 0 && AS_TYPE(returnArray->literals[i]).typeOf != ret.type) {
			interpreter->errorOutput("Bad type found in return value\n");

			//free, and skip out
			returnValue = false;
			break;
		}

		pushLiteralArray(returns, ret); //NOTE: reverses again
		freeLiteral(ret);
	}

	//manual free
	//BUGFIX: handle scopes of functions, which refer to the parent scope (leaking memory)
	while(inner.scope != AS_FUNCTION(func).scope) {
		for (int i = 0; i < inner.scope->variables.capacity; i++) {
			//handle keys, just in case
			if (IS_FUNCTION(inner.scope->variables.entries[i].key)) {
				popScope(AS_FUNCTION(inner.scope->variables.entries[i].key).scope);
				AS_FUNCTION(inner.scope->variables.entries[i].key).scope = NULL;
			}

			if (IS_FUNCTION(inner.scope->variables.entries[i].value)) {
				popScope(AS_FUNCTION(inner.scope->variables.entries[i].value).scope);
				AS_FUNCTION(inner.scope->variables.entries[i].value).scope = NULL;
			}
		}

		inner.scope = popScope(inner.scope);
	}
	freeLiteralArray(&returnsFromInner);
	freeLiteralArray(&inner.stack);
	freeLiteralArray(&inner.literalCache);

	//actual bytecode persists until next call
	return true;
}

bool callFn(Interpreter* interpreter, char* name, LiteralArray* arguments, LiteralArray* returns) {
	Literal key = TO_IDENTIFIER_LITERAL(createRefStringLength(name, strlen(name)));
	Literal val = TO_NULL_LITERAL;
	
	if (!isDelcaredScopeVariable(interpreter->scope, key)) {
		interpreter->errorOutput("No function with that name\n");
		return false;
	}

	getScopeVariable(interpreter->scope, key, &val);

	bool ret = callLiteralFn(interpreter, val, arguments, returns);

	freeLiteral(key);
	freeLiteral(val);

	return ret;
}

static bool execFnReturn(Interpreter* interpreter) {
	LiteralArray returns;
	initLiteralArray(&returns);

	//get the values of everything on the stack
	while (interpreter->stack.count > 0) {
		Literal lit = popLiteralArray(&interpreter->stack);
		if (IS_IDENTIFIER(lit)) {
			Literal idn = lit;
			parseIdentifierToValue(interpreter, &lit);
			freeLiteral(idn);
		}

		if (IS_ARRAY(lit) || IS_DICTIONARY(lit)) {
			parseCompoundToPureValues(interpreter, &lit);
		}

		pushLiteralArray(&returns, lit); //reverses the order
		freeLiteral(lit);
	}

	//and back again
	while (returns.count > 0) {
		Literal lit =  popLiteralArray(&returns);
		pushLiteralArray(&interpreter->stack, lit);
		freeLiteral(lit);
	}

	freeLiteralArray(&returns);

	//finally
	return false;
}

static bool execImport(Interpreter* interpreter) {
	Literal alias = popLiteralArray(&interpreter->stack);
	Literal identifier = popLiteralArray(&interpreter->stack);

	//access the hooks
	if (!existsLiteralDictionary(interpreter->hooks, identifier)) {
		interpreter->errorOutput("Unknown library name in import statement: ");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(alias);
		freeLiteral(identifier);
		return false;
	}

	Literal func = getLiteralDictionary(interpreter->hooks, identifier);

	if (!IS_FUNCTION_NATIVE(func)) {
		interpreter->errorOutput("Expected native function for a hook: ");
		printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		freeLiteral(func);
		freeLiteral(alias);
		freeLiteral(identifier);
		return false;
	}

	HookFn fn = (HookFn)AS_FUNCTION(func).bytecode;

	fn(interpreter, identifier, alias);

	freeLiteral(func);
	freeLiteral(alias);
	freeLiteral(identifier);
	return true;
}

static bool execIndex(Interpreter* interpreter, bool assignIntermediate) {
	//assume -> compound, first, second, third are all on the stack

	Literal third = popLiteralArray(&interpreter->stack);
	Literal second = popLiteralArray(&interpreter->stack);
	Literal first = popLiteralArray(&interpreter->stack);
	Literal compound = popLiteralArray(&interpreter->stack);

	Literal idn = compound;
	bool freeIdn = false;

	if (IS_IDENTIFIER(compound)) {
		freeIdn = true;
		if (!parseIdentifierToValue(interpreter, &compound)) {
			freeLiteral(third);
			freeLiteral(second);
			freeLiteral(first);
			freeLiteral(compound);
			//freeLiteral(idn); //since compound is freed, idn is still pointing there
			return false;
		}
	}

	if (!IS_ARRAY(compound) && !IS_DICTIONARY(compound) && !IS_STRING(compound)) {
		interpreter->errorOutput("Unknown compound found in indexing notation: ");
		printLiteralCustom(compound, interpreter->errorOutput);
		freeLiteral(third);
		freeLiteral(second);
		freeLiteral(first);
		freeLiteral(compound);
		if (freeIdn) {
			freeLiteral(idn);
		}
		return false;
	}

	//build the argument list
	LiteralArray arguments;
	initLiteralArray(&arguments);

	pushLiteralArray(&arguments, compound);
	pushLiteralArray(&arguments, first);
	pushLiteralArray(&arguments, second);
	pushLiteralArray(&arguments, third);
	pushLiteralArray(&arguments, TO_NULL_LITERAL); //it expects an assignment command
	pushLiteralArray(&arguments, TO_NULL_LITERAL); //it expects an assignment "opcode"

	//leave the idn and compound on the stack
	if (assignIntermediate) {
		if (IS_IDENTIFIER(idn)) {
			pushLiteralArray(&interpreter->stack, idn);
		}
		pushLiteralArray(&interpreter->stack, compound);
		pushLiteralArray(&interpreter->stack, first);
		pushLiteralArray(&interpreter->stack, second);
		pushLiteralArray(&interpreter->stack, third);
	}

	//call the _index function
	if (_index(interpreter, &arguments) < 0) {
		interpreter->errorOutput("Something went wrong while indexing: ");
		printLiteralCustom(idn, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		//clean up
		freeLiteral(third);
		freeLiteral(second);
		freeLiteral(first);
		freeLiteral(compound);
		if (freeIdn) {
			freeLiteral(idn);
		}
		freeLiteralArray(&arguments);
		return false;
	}

	//clean up
	freeLiteral(third);
	freeLiteral(second);
	freeLiteral(first);
	freeLiteral(compound);
	if (freeIdn) {
		freeLiteral(idn);
	}
	freeLiteralArray(&arguments);

	return true;
}

static bool execIndexAssign(Interpreter* interpreter) {
	//assume -> compound, first, second, third, assign are all on the stack

	Literal assign = popLiteralArray(&interpreter->stack);
	Literal third = popLiteralArray(&interpreter->stack);
	Literal second = popLiteralArray(&interpreter->stack);
	Literal first = popLiteralArray(&interpreter->stack);
	Literal compound = popLiteralArray(&interpreter->stack);

	Literal idn = compound;
	bool freeIdn = false;

	if (IS_IDENTIFIER(compound)) {
		freeIdn = true;
		if (!parseIdentifierToValue(interpreter, &compound)) {
			freeLiteral(assign);
			freeLiteral(third);
			freeLiteral(second);
			freeLiteral(first);
			freeLiteral(compound);
			freeLiteral(idn);
			return false;
		}
	}

	if (!IS_ARRAY(compound) && !IS_DICTIONARY(compound) && !IS_STRING(compound)) {
		interpreter->errorOutput("Unknown compound found in index assigning notation\n");
		freeLiteral(assign);
		freeLiteral(third);
		freeLiteral(second);
		freeLiteral(first);
		freeLiteral(compound);
		if (freeIdn) {
			freeLiteral(idn);
		}
		return false;
	}

	//build the opcode
	unsigned char opcode = readByte(interpreter->bytecode, &interpreter->count);
	char* opStr = "";
	switch(opcode) {
		case OP_VAR_ASSIGN:
			opStr = "=";
		break;
		case OP_VAR_ADDITION_ASSIGN:
			opStr = "+=";
		break;
		case OP_VAR_SUBTRACTION_ASSIGN:
			opStr = "-=";
		break;
		case OP_VAR_MULTIPLICATION_ASSIGN:
			opStr = "*=";
		break;
		case OP_VAR_DIVISION_ASSIGN:
			opStr = "/=";
		break;
		case OP_VAR_MODULO_ASSIGN:
			opStr = "%=";
		break;

		default:
			interpreter->errorOutput("bad opcode in index assigning notation\n");
			freeLiteral(assign);
			freeLiteral(third);
			freeLiteral(second);
			freeLiteral(first);
			freeLiteral(compound);
			if (freeIdn) {
				freeLiteral(idn);
			}
			return false;
	}

	int opLength = strlen(opStr);
	Literal op = TO_STRING_LITERAL(createRefStringLength(opStr, opLength)); //TODO: static reference optimisation?

	//build the argument list
	LiteralArray arguments;
	initLiteralArray(&arguments);

	pushLiteralArray(&arguments, compound);
	pushLiteralArray(&arguments, first);
	pushLiteralArray(&arguments, second);
	pushLiteralArray(&arguments, third);
	pushLiteralArray(&arguments, assign); //it expects an assignment command
	pushLiteralArray(&arguments, op); //it expects an assignment "opcode"

	//call the _index function
	if (_index(interpreter, &arguments) < 0) {
		//clean up
		freeLiteral(assign);
		freeLiteral(third);
		freeLiteral(second);
		freeLiteral(first);
		freeLiteral(compound);
		if (freeIdn) {
			freeLiteral(idn);
		}
		freeLiteral(op);
		freeLiteralArray(&arguments);

		return false;
	}

	//save the result (assume top of the interpreter stack is the new compound value)
	Literal result = popLiteralArray(&interpreter->stack);

	//if idn is NOT an identifier, assign backwards while there are things on the stack (inner-compound assignment, BIG assumptions here)
	if (!IS_IDENTIFIER(idn)) {
		while (interpreter->stack.count > 1) {
			//read the new values
			freeLiteral(idn);
			freeLiteral(third);
			freeLiteral(second);
			freeLiteral(first);
			freeLiteralArray(&arguments);
			initLiteralArray(&arguments);
			freeLiteral(op);

			third = popLiteralArray(&interpreter->stack);
			second = popLiteralArray(&interpreter->stack);
			first = popLiteralArray(&interpreter->stack);
			idn = popLiteralArray(&interpreter->stack);

			char* opStr = "="; //shadow, but force assignment
			int opLength = strlen(opStr);
			op = TO_STRING_LITERAL(createRefStringLength(opStr, opLength)); //TODO: static reference optimisation?

			//assign to the idn / compound - with _index
			pushLiteralArray(&arguments, idn);
			pushLiteralArray(&arguments, first);
			pushLiteralArray(&arguments, second);
			pushLiteralArray(&arguments, third);
			pushLiteralArray(&arguments, result);
			pushLiteralArray(&arguments, op);

			if (_index(interpreter, &arguments) < 0) {
				interpreter->errorOutput("Something went wrong while indexing: ");
				printLiteralCustom(idn, interpreter->errorOutput);
				interpreter->errorOutput("\n");

				//clean up
				freeLiteral(assign);
				freeLiteral(third);
				freeLiteral(second);
				freeLiteral(first);
				freeLiteral(compound);
				if (freeIdn) {
					freeLiteral(idn);
				}
				freeLiteral(op);
				freeLiteralArray(&arguments);
				freeLiteral(result);
				return false;
			}

			freeLiteral(result);
			result = popLiteralArray(&interpreter->stack);
		}

		freeLiteral(idn);
		idn = popLiteralArray(&interpreter->stack);
		compound = idn;
	}

	if (IS_IDENTIFIER(idn) && !setScopeVariable(interpreter->scope, idn, result, true)) {
		interpreter->errorOutput("Incorrect type assigned to compound member ");
		printLiteralCustom(idn, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		//clean up
		freeLiteral(assign);
		freeLiteral(third);
		freeLiteral(second);
		freeLiteral(first);
		freeLiteral(compound);
		if (freeIdn) {
			freeLiteral(idn);
		}
		freeLiteral(op);
		freeLiteralArray(&arguments);
		freeLiteral(result);
		return false;
	}

	//clean up
	freeLiteral(assign);
	freeLiteral(third);
	freeLiteral(second);
	freeLiteral(first);
	freeLiteral(compound);
	if (freeIdn) {
		freeLiteral(idn);
	}
	freeLiteral(op);
	freeLiteralArray(&arguments);
	freeLiteral(result);

	return true;
}

//the heart of toy
static void execInterpreter(Interpreter* interpreter) {
	//set the starting point for the interpreter
	if (interpreter->codeStart == -1) {
		interpreter->codeStart = interpreter->count;
	}

	unsigned char opcode = readByte(interpreter->bytecode, &interpreter->count);

	while(opcode != OP_EOF && opcode != OP_SECTION_END && !interpreter->panic) {
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

			case OP_VAR_ADDITION_ASSIGN:
			case OP_VAR_SUBTRACTION_ASSIGN:
			case OP_VAR_MULTIPLICATION_ASSIGN:
			case OP_VAR_DIVISION_ASSIGN:
			case OP_VAR_MODULO_ASSIGN:
				execVarArithmeticAssign(interpreter);
				if (!execArithmetic(interpreter, opcode)) {
					freeLiteral(popLiteralArray(&interpreter->stack));
					return;
				}
				if (!execVarAssign(interpreter)) {
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

			//TODO: custom type declarations?

			case OP_VAR_DECL:
			case OP_VAR_DECL_LONG:
				if (!execVarDecl(interpreter, opcode == OP_VAR_DECL_LONG)) {
					return;
				}
			break;

			case OP_FN_DECL:
			case OP_FN_DECL_LONG:
				if (!execFnDecl(interpreter, opcode == OP_FN_DECL_LONG)) {
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

			case OP_TYPE_OF:
				if (!execTypeOf(interpreter)) {
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

			case OP_AND:
				if (!execAnd(interpreter)) {
					return;
				}
			break;

			case OP_OR:
				if (!execOr(interpreter)) {
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

			case OP_FN_CALL:
				if (!execFnCall(interpreter, false)) {
					return;
				}
			break;

			case OP_DOT:
				if (!execFnCall(interpreter, true)) { //compensate for the out-of-order arguments
					return;
				}
			break;


			case OP_FN_RETURN:
				if (!execFnReturn(interpreter)) {
					return;
				}
			break;

			case OP_IMPORT:
				if (!execImport(interpreter)) {
					return;
				}
			break;

			case OP_INDEX:
				if (!execIndex(interpreter, false)) {
					return;
				}
			break;

			case OP_INDEX_ASSIGN_INTERMEDIATE:
				if (!execIndex(interpreter, true)) {
					return;
				}
			break;

			case OP_INDEX_ASSIGN:
				if (!execIndexAssign(interpreter)) {
					return;
				}
			break;

			case OP_POP_STACK:
				while (interpreter->stack.count > 0) {
					freeLiteral(popLiteralArray(&interpreter->stack));
				}
			break;

			default:
				interpreter->errorOutput("Unknown opcode found, terminating\n");
				return;
		}

		opcode = readByte(interpreter->bytecode, &interpreter->count);
	}
}

static void readInterpreterSections(Interpreter* interpreter) {
	//data section
	const unsigned short literalCount = readShort(interpreter->bytecode, &interpreter->count);

#ifndef TOY_EXPORT
	if (command.verbose) {
		printf(NOTICE "Reading %d literals\n" RESET, literalCount);
	}
#endif

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = readByte(interpreter->bytecode, &interpreter->count);

		switch(literalType) {
			case LITERAL_NULL:
				//read the null
				pushLiteralArray(&interpreter->literalCache, TO_NULL_LITERAL);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(null)\n");
				}
#endif
			break;

			case LITERAL_BOOLEAN: {
				//read the booleans
				const bool b = readByte(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_BOOLEAN_LITERAL(b);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(boolean %s)\n", b ? "true" : "false");
				}
#endif
			}
			break;

			case LITERAL_INTEGER: {
				const int d = readInt(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_INTEGER_LITERAL(d);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(integer %d)\n", d);
				}
#endif
			}
			break;

			case LITERAL_FLOAT: {
				const float f = readFloat(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_FLOAT_LITERAL(f);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(float %f)\n", f);
				}
#endif
			}
			break;

			case LITERAL_STRING: {
				char* s = readString(interpreter->bytecode, &interpreter->count);
				int length = strlen(s);
				Literal literal = TO_STRING_LITERAL(createRefStringLength(s, length));
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(string \"%s\")\n", s);
				}
#endif
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

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(array ");
					Literal literal = TO_ARRAY_LITERAL(array);
					printLiteral(literal);
					printf(")\n");
				}
#endif

				//finally, push the array proper
				Literal literal = TO_ARRAY_LITERAL(array);
				pushLiteralArray(&interpreter->literalCache, literal); //copied

				freeLiteralArray(array);
				FREE(LiteralArray, array);
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

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(dictionary ");
					Literal literal = TO_DICTIONARY_LITERAL(dictionary);
					printLiteral(literal);
					printf(")\n");
				}
#endif

				//finally, push the dictionary proper
				Literal literal = TO_DICTIONARY_LITERAL(dictionary);
				pushLiteralArray(&interpreter->literalCache, literal); //copied

				freeLiteralDictionary(dictionary);
				FREE(LiteralDictionary, dictionary);
			}
			break;

			case LITERAL_FUNCTION: {
				//read the index
				unsigned short index = readShort(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_INTEGER_LITERAL(index);

				//change the type, to read it PROPERLY below
				literal.type = LITERAL_FUNCTION_INTERMEDIATE;

				//push to the literal cache
				pushLiteralArray(&interpreter->literalCache, literal);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(function)\n");
				}
#endif
			}
			break;

			case LITERAL_IDENTIFIER: {
				char* str = readString(interpreter->bytecode, &interpreter->count);

				int length = strlen(str);
				Literal identifier = TO_IDENTIFIER_LITERAL(createRefStringLength(str, length));

				pushLiteralArray(&interpreter->literalCache, identifier);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(identifier %s (hash: %x))\n", toCString(AS_IDENTIFIER(identifier)), identifier.as.identifier.hash);
				}
#endif

				freeLiteral(identifier);
			}
			break;

			case LITERAL_TYPE: {
				//what the literal is
				LiteralType literalType = (LiteralType)readByte(interpreter->bytecode, &interpreter->count);
				unsigned char constant = readByte(interpreter->bytecode, &interpreter->count);

				Literal typeLiteral = TO_TYPE_LITERAL(literalType, constant);

				//save the type
				pushLiteralArray(&interpreter->literalCache, typeLiteral);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}
#endif
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

					TYPE_PUSH_SUBTYPE(&typeLiteral, copyLiteral(interpreter->literalCache.literals[vt]));
				}

				if (AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY) {
					unsigned short kt = readShort(interpreter->bytecode, &interpreter->count);
					unsigned short vt = readShort(interpreter->bytecode, &interpreter->count);

					TYPE_PUSH_SUBTYPE(&typeLiteral, copyLiteral(interpreter->literalCache.literals[kt]));
					TYPE_PUSH_SUBTYPE(&typeLiteral, copyLiteral(interpreter->literalCache.literals[vt]));
				}

				//save the type
				pushLiteralArray(&interpreter->literalCache, typeLiteral); //copied

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}
#endif

				freeLiteral(typeLiteral);
			}
			break;

			case LITERAL_INDEX_BLANK:
				//read the blank
				pushLiteralArray(&interpreter->literalCache, TO_INDEX_BLANK_LITERAL);

#ifndef TOY_EXPORT
				if (command.verbose) {
					printf("(blank)\n");
				}
#endif
			break;
		}
	}

	consumeByte(interpreter, OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the literal section

	//read the function metadata
	int functionCount = readShort(interpreter->bytecode, &interpreter->count);
	int functionSize = readShort(interpreter->bytecode, &interpreter->count); //might not be needed

	//read in the functions
	for (int i = 0; i < interpreter->literalCache.count; i++) {
		if (interpreter->literalCache.literals[i].type == LITERAL_FUNCTION_INTERMEDIATE) {
			//get the size of the function
			size_t size = (size_t)readShort(interpreter->bytecode, &interpreter->count);

			//read the function code (literal cache and all)
			unsigned char* bytes = ALLOCATE(unsigned char, size);
			memcpy(bytes, interpreter->bytecode + interpreter->count, size); //TODO: -1 for the ending mark
			interpreter->count += size;

			//assert that the last memory slot is function end
			if (bytes[size - 1] != OP_FN_END) {
				interpreter->errorOutput("[internal] Failed to find function end");
				FREE_ARRAY(unsigned char, bytes, size);
				return;
			}

			//change the type to normal
			interpreter->literalCache.literals[i] = TO_FUNCTION_LITERAL(bytes, size);
			AS_FUNCTION(interpreter->literalCache.literals[i]).scope = NULL;
		}
	}

	consumeByte(interpreter, OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the function section
}

//exposed functions
void initInterpreter(Interpreter* interpreter) {
	interpreter->hooks = ALLOCATE(LiteralDictionary, 1);
	initLiteralDictionary(interpreter->hooks);

	//set up the output streams
	setInterpreterPrint(interpreter, printWrapper);
	setInterpreterAssert(interpreter, assertWrapper);
	setInterpreterError(interpreter, errorWrapper);

	interpreter->scope = NULL;
	resetInterpreter(interpreter);
}

void runInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length) {
	//initialize here instead of initInterpreter()
	initLiteralArray(&interpreter->literalCache);
	interpreter->bytecode = NULL;
	interpreter->length = 0;
	interpreter->count = 0;
	interpreter->codeStart = -1;

	initLiteralArray(&interpreter->stack);

	interpreter->depth = 0;
	interpreter->panic = false;

	//prep the bytecode
	interpreter->bytecode = bytecode;
	interpreter->length = length;
	interpreter->count = 0;

	if (!interpreter->bytecode) {
		interpreter->errorOutput("No valid bytecode given\n");
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

	if (major != TOY_VERSION_MAJOR || minor > TOY_VERSION_MINOR) {
		char buffer[MAX_STRING_LENGTH];
		snprintf(buffer, MAX_STRING_LENGTH, "Interpreter/bytecode version mismatch (expected %d.%d.%d or earlier, given %d.%d.%d)\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, major, minor, patch);
		interpreter->errorOutput(buffer);
		return;
	}

	const char* build = readString(interpreter->bytecode, &interpreter->count);

#ifndef TOY_EXPORT
	if (command.verbose) {
		if (strncmp(build, TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD))) {
			printf(WARN "Warning: interpreter/bytecode build mismatch\n" RESET);
		}
	}
#endif

	consumeByte(interpreter, OP_SECTION_END, interpreter->bytecode, &interpreter->count);

	//read the sections of the bytecode
	readInterpreterSections(interpreter);

	//code section
#ifndef TOY_EXPORT
	if (command.verbose) {
		printf(NOTICE "executing bytecode\n" RESET);
	}
#endif

	//execute the interpreter
	execInterpreter(interpreter);

	//BUGFIX: clear the stack (for repl - stack must be balanced)
	while(interpreter->stack.count > 0) {
		Literal lit = popLiteralArray(&interpreter->stack);
		freeLiteral(lit);
	}

	//free the bytecode immediately after use TODO: because why?
	FREE_ARRAY(unsigned char, interpreter->bytecode, interpreter->length);

	//free the associated data
	freeLiteralArray(&interpreter->literalCache);
	freeLiteralArray(&interpreter->stack);
}

void resetInterpreter(Interpreter* interpreter) {
	//free the interpreter scope
	while(interpreter->scope != NULL) {
		interpreter->scope = popScope(interpreter->scope);
	}

	//prep the scope
	interpreter->scope = pushScope(NULL);

	//globally available functions
	injectNativeFn(interpreter, "_set", _set);
	injectNativeFn(interpreter, "_get", _get);
	injectNativeFn(interpreter, "_push", _push);
	injectNativeFn(interpreter, "_pop", _pop);
	injectNativeFn(interpreter, "_length", _length);
	injectNativeFn(interpreter, "_clear", _clear);
}

void freeInterpreter(Interpreter* interpreter) {
	//free the interpreter scope
	while(interpreter->scope != NULL) {
		interpreter->scope = popScope(interpreter->scope);
	}

	if (interpreter->hooks) {
		freeLiteralDictionary(interpreter->hooks);
		FREE(LiteralDictionary, interpreter->hooks);
	}

	interpreter->hooks = NULL;
}
