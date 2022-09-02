#include "interpreter.h"
#include "console_colors.h"

#include "common.h"
#include "memory.h"
#include "keyword_types.h"

#include <stdio.h>
#include <string.h>

static void stdoutWrapper(const char* output) {
	printf("%s", output);
	printf("\n"); //default new line
}

static void stderrWrapper(const char* output) {
	fprintf(stderr, ERROR "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n" RESET); //default new line
}

bool injectNativeFn(Interpreter* interpreter, char* name, NativeFn func) {
	//reject reserved words
	if (findTypeByKeyword(name) != TOKEN_EOF) {
		printf("Can't override an existing keyword");
		return false;
	}

	Literal identifier = TO_IDENTIFIER_LITERAL(copyString(name, strlen(name)), strlen(name));

	//make sure the name isn't taken
	if (existsLiteralDictionary(&interpreter->scope->variables, identifier)) {
		printf("Can't override an existing variable");
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

bool parseIdentifierToValue(Interpreter* interpreter, Literal* literalPtr) {
	//this converts identifiers to values
	if (IS_IDENTIFIER(*literalPtr)) {
	//	Literal idn = *literalPtr;
		if (!getScopeVariable(interpreter->scope, *literalPtr, literalPtr)) {
			printf(ERROR "ERROR: Undeclared variable \"");;
			printLiteral(*literalPtr);
			printf("\"\n" RESET);
			return false;
		}
	//	freeLiteral(idn);
	}

	return true;
}

int _set(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 3) {
		(interpreter->printOutput)("Incorrect number of arguments to _set");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];
	Literal key = arguments->literals[1];
	Literal val = arguments->literals[2];

	if (!IS_IDENTIFIER(idn)) {
		(interpreter->printOutput)("expected identifier in _set");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);
	parseIdentifierToValue(interpreter, &key);
	parseIdentifierToValue(interpreter, &val);

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal typeLiteral = getScopeType(interpreter->scope, key);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY) {
				Literal subtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];

				if (AS_TYPE(subtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(subtypeLiteral).typeOf != val.type) {
					(interpreter->printOutput)("bad argument type in _set");
					return -1;
				}
			}

			if (!IS_INTEGER(key)) {
				(interpreter->printOutput)("Expected integer index in _set");
				return -1;
			}

			if (AS_ARRAY(obj)->count <= AS_INTEGER(key) || AS_INTEGER(key) < 0) {
				(interpreter->printOutput)("Index out of bounds in _set");
				return -1;
			}

			//don't use pushLiteralArray, since we're setting
			AS_ARRAY(obj)->literals[AS_INTEGER(key)] = copyLiteral(val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				printf(ERROR "ERROR: Incorrect type assigned to array (_set) \"");
				printLiteral(val);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);
			freeLiteral(key);
			freeLiteral(val);

			return 0;
		}

		case LITERAL_DICTIONARY: {
			Literal typeLiteral = getScopeType(interpreter->scope, key);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY) {
				Literal keySubtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];
				Literal valSubtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[1];

				if (AS_TYPE(keySubtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(keySubtypeLiteral).typeOf != key.type) {
					(interpreter->printOutput)("bad argument type in _set");
					return -1;
				}

				if (AS_TYPE(valSubtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(valSubtypeLiteral).typeOf != val.type) {
					(interpreter->printOutput)("bad argument type in _set");
					return -1;
				}
			}

			setLiteralDictionary(AS_DICTIONARY(obj), key, val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				printf(ERROR "ERROR: Incorrect type assigned to dictionary (_get) \"");
				printLiteral(val);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);
			freeLiteral(key);
			freeLiteral(val);

			return 0;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _set");
			printLiteral(obj);
			return -1;
	}
}

int _get(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		(interpreter->printOutput)("Incorrect number of arguments to _get");
		return -1;
	}

	Literal obj = arguments->literals[0];
	Literal key = arguments->literals[1];

	parseIdentifierToValue(interpreter, &obj);

	switch(obj.type) {
		case LITERAL_ARRAY: {
			if (!IS_INTEGER(key)) {
				(interpreter->printOutput)("Expected integer index in _get");
				return -1;
			}

			if (AS_ARRAY(obj)->count <= AS_INTEGER(key) || AS_INTEGER(key) < 0) {
				(interpreter->printOutput)("Index out of bounds in _get");
				return -1;
			}

			pushLiteralArray(&interpreter->stack, AS_ARRAY(obj)->literals[AS_INTEGER(key)]);
			freeLiteral(obj);
			return 1;
		}

		case LITERAL_DICTIONARY: {
			Literal dict = getLiteralDictionary(AS_DICTIONARY(obj), key);
			pushLiteralArray(&interpreter->stack, dict);
			freeLiteral(dict);
			freeLiteral(obj);
			return 1;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _get ");
			printLiteral(obj);
			return -1;
	}
}

int _push(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		(interpreter->printOutput)("Incorrect number of arguments to _push");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];
	Literal val = arguments->literals[1];

	if (!IS_IDENTIFIER(idn)) {
		(interpreter->printOutput)("expected identifier in _push");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);
	parseIdentifierToValue(interpreter, &val);

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal typeLiteral = getScopeType(interpreter->scope, val);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY) {
				Literal subtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];

				if (AS_TYPE(subtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(subtypeLiteral).typeOf != val.type) {
					(interpreter->printOutput)("bad argument type in _push");
					return -1;
				}
			}

			pushLiteralArray(AS_ARRAY(obj), val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				printf(ERROR "ERROR: Incorrect type assigned to array (_push) \"");
				printLiteral(val);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);

			return 0;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _push ");
			printLiteral(obj);
			return -1;
	}
}

int _pop(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		(interpreter->printOutput)("Incorrect number of arguments to _pop");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];

	if (!IS_IDENTIFIER(idn)) {
		(interpreter->printOutput)("expected identifier in _pop");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal lit = popLiteralArray(AS_ARRAY(obj));
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				printf(ERROR "ERROR: Incorrect type assigned to array (_pop) \"");
				printLiteral(obj);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);

			return 1;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _pop ");
			printLiteral(obj);
			return -1;
	}
}

int _length(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		(interpreter->printOutput)("Incorrect number of arguments to _get");
		return -1;
	}

	Literal obj = arguments->literals[0];

	bool freeObj = false;
	if (IS_IDENTIFIER(obj)) {
		parseIdentifierToValue(interpreter, &obj);
		freeObj = true;
	}

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal lit = TO_INTEGER_LITERAL( AS_ARRAY(obj)->count );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		case LITERAL_DICTIONARY: {
			Literal lit = TO_INTEGER_LITERAL( AS_DICTIONARY(obj)->count );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		case LITERAL_STRING: {
			Literal lit = TO_INTEGER_LITERAL( strlen(AS_STRING(obj)) );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _length ");
			printLiteral(obj);
			return -1;
	}

	if (freeObj) {
		freeLiteral(obj);
	}

	return 1;
}

int _clear(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		(interpreter->printOutput)("Incorrect number of arguments to _get");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];

	if (!IS_IDENTIFIER(idn)) {
		(interpreter->printOutput)("expected identifier in _clear");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	//NOTE: just pass in new compounds

	switch(obj.type) {
		case LITERAL_ARRAY: {
			LiteralArray* array = ALLOCATE(LiteralArray, 1);
			initLiteralArray(array);

			Literal obj = TO_ARRAY_LITERAL(array);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				printf(ERROR "ERROR: Incorrect type assigned to array (_clear) \"");
				printLiteral(obj);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);

			break;
		}

		case LITERAL_DICTIONARY: {
			LiteralDictionary* dictionary = ALLOCATE(LiteralDictionary, 1);
			initLiteralDictionary(dictionary);

			Literal obj = TO_DICTIONARY_LITERAL(dictionary);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				printf(ERROR "ERROR: Incorrect type assigned to dictionary (_clear) \"");
				printLiteral(obj);
				printf("\"\n" RESET);
				return -1;
			}

			freeLiteral(obj);

			break;
		}

		default:
			(interpreter->printOutput)("Incorrect compound type in _clear ");
			printLiteral(obj);
			return -1;
	}

	freeLiteral(obj);
	return 1;
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

	//globally available functions (tmp?)
	injectNativeFn(interpreter, "_set", _set);
	injectNativeFn(interpreter, "_get", _get);
	injectNativeFn(interpreter, "_push", _push);
	injectNativeFn(interpreter, "_pop", _pop);
	injectNativeFn(interpreter, "_length", _length);
	injectNativeFn(interpreter, "_clear", _clear);

	interpreter->panic = false;
}

void freeInterpreter(Interpreter* interpreter) {
	//BUGFIX: handle scopes of functions, which refer to the parent scope (leaking memory)
	for (int i = 0; i < interpreter->scope->variables.capacity; i++) {
		//handle keys, just in case
		if (IS_FUNCTION(interpreter->scope->variables.entries[i].key)) {
			popScope(AS_FUNCTION(interpreter->scope->variables.entries[i].key).scope);
			AS_FUNCTION(interpreter->scope->variables.entries[i].key).scope = NULL;
		}

		if (IS_FUNCTION(interpreter->scope->variables.entries[i].value)) {
			popScope(AS_FUNCTION(interpreter->scope->variables.entries[i].value).scope);
			AS_FUNCTION(interpreter->scope->variables.entries[i].value).scope = NULL;
		}
	}

	popScope(interpreter->scope);
	interpreter->scope = NULL;

	freeLiteralArray(&interpreter->literalCache);
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
	parseIdentifierToValue(interpreter, &lhs);

	if (!IS_STRING(rhs)) {
		printf(ERROR "ERROR: The assert keyword needs a string as the second argument, received: ");
		printLiteral(rhs);
		printf("\n" RESET);
		return false;
	}

	if (IS_NULL(lhs) || !IS_TRUTHY(lhs)) {
		(*interpreter->assertOutput)(AS_STRING(rhs));
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

	freeLiteral(lit);

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
		printf(ERROR "[internal] The interpreter can't negate that literal: ");
		printLiteral(lit);
		printf("\n" RESET);

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

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	if (IS_BOOLEAN(lit)) {
		lit = TO_BOOLEAN_LITERAL(!AS_BOOLEAN(lit));
	}
	else {
		printf(ERROR "[internal] The interpreter can't invert that literal: ");
		printLiteral(lit);
		printf("\n" RESET);

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
		if (strlen(AS_STRING(lhs)) + strlen(AS_STRING(rhs)) > MAX_STRING_LENGTH) {
			printf(ERROR "ERROR: Can't concatenate these strings (result is too long)\n" RESET);
			return false;
		}

		//concat the strings
		char buffer[MAX_STRING_LENGTH];
		snprintf(buffer, MAX_STRING_LENGTH, "%s%s", AS_STRING(lhs), AS_STRING(rhs));
		Literal literal = TO_STRING_LITERAL( copyString(buffer, strlen(buffer)), strlen(buffer) );
		pushLiteralArray(&interpreter->stack, literal);
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
					printf(ERROR "ERROR: Can't divide by zero (error found in interpreter)" RESET);
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_INTEGER_LITERAL( AS_INTEGER(lhs) / AS_INTEGER(rhs) ));
				return true;

			case OP_MODULO:
			case OP_VAR_MODULO_ASSIGN:
				if (AS_INTEGER(rhs) == 0) {
					printf(ERROR "ERROR: Can't modulo by zero (error found in interpreter)" RESET);
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
	if (opcode == OP_MODULO || opcode == OP_VAR_MODULO_ASSIGN) {
		printf(ERROR "ERROR: Bad arithmetic argument (modulo on floats not allowed)\n" RESET);
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
					printf(ERROR "ERROR: Can't divide by zero (error found in interpreter)" RESET);
					return false;
				}
				pushLiteralArray(&interpreter->stack, TO_FLOAT_LITERAL( AS_FLOAT(lhs) / AS_FLOAT(rhs) ));
				return true;

			default:
				printf(ERROR "[internal] bad opcode argument passed to execArithmetic()" RESET);
				return false;
		}
	}

	//wrong types
	printf(ERROR "ERROR: Bad arithmetic argument ");
	printLiteral(lhs);
	printf(" and ");
	printLiteral(rhs);
	printf("\n" RESET);

	freeLiteral(lhs);
	freeLiteral(rhs);

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

	bool freeType = false;
	if (IS_IDENTIFIER(type)) {
		parseIdentifierToValue(interpreter, &type);
		freeType = true;
	}

	if (!declareScopeVariable(interpreter->scope, identifier, type)) {
		printf(ERROR "ERROR: Can't redefine the variable \"");
		printLiteral(identifier);
		printf("\"\n" RESET);
		return false;
	}

	Literal val = popLiteralArray(&interpreter->stack);
	parseIdentifierToValue(interpreter, &val);

	if (!IS_NULL(val) && !setScopeVariable(interpreter->scope, identifier, val, false)) {
		printf(ERROR "ERROR: Incorrect type assigned to variable \"");
		printLiteral(identifier);
		printf("\"\n" RESET);

		freeLiteral(identifier); //TODO: test this
		freeLiteral(type);
		freeLiteral(val);

		return false;
	}

	freeLiteral(val);

	if (freeType) {
		freeLiteral(type);
	}

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
		printf(ERROR "ERROR: Can't redefine the function \"");
		printLiteral(identifier);
		printf("\"\n" RESET);
		return false;
	}

	if (!setScopeVariable(interpreter->scope, identifier, function, false)) { //scope gets copied here
		printf(ERROR "ERROR: Incorrect type assigned to variable \"");
		printLiteral(identifier);
		printf("\"\n" RESET);
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

	parseIdentifierToValue(interpreter, &rhs);

	if (!IS_IDENTIFIER(lhs)) {
		printf(ERROR "ERROR: Can't assign to a non-variable \"");
		printLiteral(lhs);
		printf("\"\n" RESET);
		return false;
	}

	if (!isDelcaredScopeVariable(interpreter->scope, lhs)) {
		printf(ERROR "ERROR: Undeclared variable \"");
		printLiteral(lhs);
		printf("\"\n" RESET);
		return false;
	}

	if (!setScopeVariable(interpreter->scope, lhs, rhs, true)) {
		printf(ERROR "ERROR Incorrect type assigned to variable \"");
		printLiteral(lhs);
		printf("\"\n" RESET);
		return false;
	}

	freeLiteral(lhs);
	freeLiteral(rhs);

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

	if (!parseIdentifierToValue(interpreter, &value)) {
		return false;
	}

	Literal result = TO_NULL_LITERAL;

	if (IS_NULL(value)) {
		printf(ERROR "ERROR: Can't cast a null value\n" RESET);

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
				char* str = AS_BOOLEAN(value) ? "true" : "false";

				result = TO_STRING_LITERAL(copyString(str, strlen(str)), strlen(str));
			}

			if (IS_INTEGER(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%d", AS_INTEGER(value));
				result = TO_STRING_LITERAL(copyString(buffer, strlen(buffer)), strlen(buffer));
			}

			if (IS_FLOAT(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%g", AS_FLOAT(value));
				result = TO_STRING_LITERAL(copyString(buffer, strlen(buffer)), strlen(buffer));
			}
		break;

		default:
			printf(ERROR"ERROR: Unknown cast type found %d, terminating\n" RESET, AS_TYPE(type).typeOf);
			return false;
	}

	//leave the new value on the stack
	pushLiteralArray(&interpreter->stack, result);

	freeLiteral(result);
	freeLiteral(value);
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
		printf(ERROR "ERROR: Incorrect type in comparison, value \"");
		printLiteral(lhs);
		printf("\"\n" RESET);
		freeLiteral(lhs);
		freeLiteral(rhs);
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		printf(ERROR "ERROR: Incorrect type in comparison, value \"");
		printLiteral(rhs);
		printf("\"\n" RESET);
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
		printf(ERROR "ERROR: Incorrect type in comparison, value \"");
		printLiteral(lhs);
		printf("\"\n" RESET);
		freeLiteral(lhs);
		freeLiteral(rhs);
		return false;
	}

	if (!(IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
		printf(ERROR "ERROR: Incorrect type in comparison, value \"");
		printLiteral(rhs);
		printf("\"\n" RESET);
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
		printf(ERROR "[internal] Jump out of range\n" RESET);
		return false;
	}

	//actually jump
	interpreter->count = target + interpreter->codeStart;

	return true;
}

static bool execFalseJump(Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		printf(ERROR "[internal] Jump out of range (false jump)\n" RESET);
		return false;
	}

	//actually jump
	Literal lit = popLiteralArray(&interpreter->stack);

	if (!parseIdentifierToValue(interpreter, &lit)) {
		return false;
	}

	if (IS_NULL(lit)) {
		printf(ERROR "Error: Null detected in comparison\n" RESET);
		return false;
	}

	if (!IS_TRUTHY(lit)) {
		interpreter->count = target + interpreter->codeStart;
	}

	freeLiteral(lit);

	return true;
}

//forward declare
static void execInterpreter(Interpreter*);
static void readInterpreterSections(Interpreter* interpreter);

static bool execFnCall(Interpreter* interpreter) {
	LiteralArray arguments;
	initLiteralArray(&arguments);

	Literal stackSize = popLiteralArray(&interpreter->stack);

	//unpack the stack of arguments
	for (int i = 0; i < AS_INTEGER(stackSize); i++) {
		Literal lit = popLiteralArray(&interpreter->stack);
		pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	Literal identifier = popLiteralArray(&interpreter->stack);

	Literal func = identifier;

	if (!parseIdentifierToValue(interpreter, &func)) {
		freeLiteralArray(&arguments);
		freeLiteral(identifier);
		return false;
	}

	freeLiteral(identifier);

	//check for side-loaded native functions
	if (IS_FUNCTION_NATIVE(func)) {
		//reverse the order to the correct order
		LiteralArray correct;
		initLiteralArray(&correct);

		while(arguments.count) {
			pushLiteralArray(&correct, popLiteralArray(&arguments));
		}

		freeLiteralArray(&arguments);

		//call the native function
		((NativeFn) AS_FUNCTION(func).bytecode )(interpreter, &correct);

		freeLiteralArray(&correct);
		return true;
	}

	//set up a new interpreter
	Interpreter inner;

	//init the inner interpreter manually
	initLiteralArray(&inner.literalCache);
	inner.scope = pushScope(func.as.function.scope);
	inner.bytecode = AS_FUNCTION(func).bytecode;
	inner.length = func.as.function.length;
	inner.count = 0;
	inner.panic = false;
	initLiteralArray(&inner.stack);
	setInterpreterPrint(&inner, interpreter->printOutput);
	setInterpreterAssert(&inner, interpreter->assertOutput);

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
	if ((IS_NULL(restParam) && paramArray->count != arguments.count * 2) || (!IS_NULL(restParam) && paramArray->count -2 > arguments.count * 2)) {
		printf(ERROR "ERROR: Incorrect number of arguments passed to function \"");
		printLiteral(identifier);
		printf("\"\n" RESET);

		//free, and skip out
		freeLiteralArray(&arguments);
		popScope(inner.scope);
		freeInterpreter(&inner);

		return false;
	}

	//contents is the indexes of identifier & type
	for (int i = 0; i < paramArray->count - (IS_NULL(restParam) ? 0 : 2); i += 2) { //don't count the rest parameter, if present
		//declare and define each entry in the scope
		if (!declareScopeVariable(inner.scope, paramArray->literals[i], paramArray->literals[i + 1])) {
			printf(ERROR "[internal] Could not re-declare parameter\n" RESET);
			//free, and skip out
			freeLiteralArray(&arguments);
			popScope(inner.scope);
			freeInterpreter(&inner);
			return false;
		}

		if (!setScopeVariable(inner.scope, paramArray->literals[i], popLiteralArray(&arguments), false)) {
			printf(ERROR "[internal] Could not define parameter (bad type?)\n" RESET);
			//free, and skip out
			freeLiteralArray(&arguments);
			popScope(inner.scope);
			freeInterpreter(&inner);
			return false;
		}
	}

	//if using rest, pack the optional extra arguments into the rest parameter (array)
	if (!IS_NULL(restParam)) {
		LiteralArray rest;
		initLiteralArray(&rest);

		while (arguments.count > 0) {
			Literal lit = popLiteralArray(&arguments);
			pushLiteralArray(&rest, lit);
			freeLiteral(lit);
		}

		Literal restType = TO_TYPE_LITERAL(LITERAL_ARRAY, true);
		Literal any = TO_TYPE_LITERAL(LITERAL_ANY, false);
		TYPE_PUSH_SUBTYPE(&restType, any);

		//declare & define the rest parameter
		if (!declareScopeVariable(inner.scope, restParam, restType)) {
			printf(ERROR "[internal] Could not declare rest parameter\n" RESET);
			//free, and skip out
			freeLiteral(restType);
			freeLiteralArray(&rest);
			freeLiteralArray(&arguments);
			popScope(inner.scope);
			freeInterpreter(&inner);
			return false;
		}

		if (!setScopeVariable(inner.scope, restParam, TO_ARRAY_LITERAL(&rest), false)) {
			printf(ERROR "[internal] Could not define rest parameter\n" RESET);
			//free, and skip out
			freeLiteral(restType);
			freeLiteralArray(&rest);
			freeLiteralArray(&arguments);
			popScope(inner.scope);
			freeInterpreter(&inner);
			return false;
		}

		freeLiteral(restType);
	}

	//execute the interpreter
	execInterpreter(&inner);

	//adopt the panic state
	interpreter->panic = inner.panic;

	//accept the stack as the results
	LiteralArray returns;
	initLiteralArray(&returns);

	//unpack the results
	while (inner.stack.count > 0) {
		Literal lit = popLiteralArray(&inner.stack);
		pushLiteralArray(&returns, lit); //NOTE: also reverses the order
		freeLiteral(lit);
	}

	bool returnValue = true;

	//TODO: remove this when multiple assignment is enabled - note the BUGFIX that balances the stack
	if (returns.count > 1) {
		printf(ERROR "ERROR: Too many values returned (multiple returns not yet implemented)\n" RESET);

		returnValue = false;
	}

	for (int i = 0; i < returns.count && returnValue; i++) {
		Literal ret = popLiteralArray(&returns);

		//check the return types
		if (returnArray->count > 0 && AS_TYPE(returnArray->literals[i]).typeOf != ret.type) {
			printf(ERROR "ERROR: bad type found in return value\n" RESET);

			//free, and skip out
			returnValue = false;
			break;
		}

		pushLiteralArray(&interpreter->stack, ret); //NOTE: reverses again
		freeLiteral(ret);
	}

	//free
	//BUGFIX: handle scopes of functions, which refer to the parent scope (leaking memory)
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
	freeLiteralArray(&returns);
	freeLiteralArray(&arguments);
	freeLiteralArray(&inner.stack);
	freeLiteralArray(&inner.literalCache);
	popScope(inner.scope);
	freeLiteral(func);

	//actual bytecode persists until next call
	return true;
}

static bool execFnReturn(Interpreter* interpreter) {
	LiteralArray returns;
	initLiteralArray(&returns);

	//get the values of everything on the stack
	while (interpreter->stack.count > 0) {
		Literal lit = popLiteralArray(&interpreter->stack);
		parseIdentifierToValue(interpreter, &lit);
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

//the heart of toy
static void execInterpreter(Interpreter* interpreter) {
	//set the starting point for the interpreter
	interpreter->codeStart = interpreter->count;

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
				if (!execFnCall(interpreter)) {
					return;
				}
			break;

			case OP_FN_RETURN:
				if (!execFnReturn(interpreter)) {
					return;
				}
			break;

			default:
				printf(ERROR "Error: Unknown opcode found %d, terminating\n" RESET, opcode);
				printLiteralArray(&interpreter->stack, "\n");
				return;
		}

		opcode = readByte(interpreter->bytecode, &interpreter->count);
	}
}

static void readInterpreterSections(Interpreter* interpreter) {
	//data section
	const unsigned short literalCount = readShort(interpreter->bytecode, &interpreter->count);

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
				Literal literal = TO_BOOLEAN_LITERAL(b);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

				if (command.verbose) {
					printf("(boolean %s)\n", b ? "true" : "false");
				}
			}
			break;

			case LITERAL_INTEGER: {
				const int d = readInt(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_INTEGER_LITERAL(d);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

				if (command.verbose) {
					printf("(integer %d)\n", d);
				}
			}
			break;

			case LITERAL_FLOAT: {
				const float f = readFloat(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_FLOAT_LITERAL(f);
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

				if (command.verbose) {
					printf("(float %f)\n", f);
				}
			}
			break;

			case LITERAL_STRING: {
				char* s = readString(interpreter->bytecode, &interpreter->count);
				Literal literal = TO_STRING_LITERAL( copyString(s, strlen(s)), strlen(s) );
				pushLiteralArray(&interpreter->literalCache, literal);
				freeLiteral(literal);

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
					Literal literal = TO_ARRAY_LITERAL(array);
					printLiteral(literal);
					printf(")\n");
				}

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

				if (command.verbose) {
					printf("(dictionary ");
					Literal literal = TO_DICTIONARY_LITERAL(dictionary);
					printLiteral(literal);
					printf(")\n");
				}

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

				if (command.verbose) {
					printf("(function)\n");
				}
			}
			break;

			case LITERAL_IDENTIFIER: {
				char* str = readString(interpreter->bytecode, &interpreter->count);

				Literal identifier = TO_IDENTIFIER_LITERAL(copyString(str, strlen(str)), strlen(str));

				pushLiteralArray(&interpreter->literalCache, identifier);

				if (command.verbose) {
					printf("(identifier %s (hash: %x))\n", AS_IDENTIFIER(identifier), identifier.as.identifier.hash);
				}

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

				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}

//				freeLiteral(typeLiteral);
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

					TYPE_PUSH_SUBTYPE(&typeLiteral, copyLiteral(interpreter->literalCache.literals[kt]));
					TYPE_PUSH_SUBTYPE(&typeLiteral, copyLiteral(interpreter->literalCache.literals[vt]));
				}

				//save the type
				pushLiteralArray(&interpreter->literalCache, typeLiteral); //copied

				if (command.verbose) {
					printf("(type ");
					printLiteral(typeLiteral);
					printf(")\n");
				}

				freeLiteral(typeLiteral);
			}
			break;
		}
	}

	consumeByte(OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the literal section

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
				printf(ERROR "[internal] Failed to find function end" RESET);
				FREE_ARRAY(unsigned char, bytes, size);
				return;
			}

			//change the type to normal
			interpreter->literalCache.literals[i] = TO_FUNCTION_LITERAL(bytes, size);
			AS_FUNCTION(interpreter->literalCache.literals[i]).scope = pushScope(interpreter->scope); //BUGFIX
		}
	}

	consumeByte(OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the function section
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

	//read the sections of the bytecode
	readInterpreterSections(interpreter);

	//code section
	if (command.verbose) {
		printf(NOTICE "executing bytecode\n" RESET);
	}

	//execute the interpreter
	execInterpreter(interpreter);

	//BUGFIX: clear the stack (for repl - stack must be balanced)
	while(interpreter->stack.count > 0) {
		Literal lit = popLiteralArray(&interpreter->stack);
		freeLiteral(lit);
	}

	//free the bytecode immediately after use
	FREE_ARRAY(unsigned char, interpreter->bytecode, interpreter->length);
}
