#include "toy_interpreter.h"
#include "toy_console_colors.h"

#include "toy_common.h"
#include "toy_memory.h"
#include "toy_keyword_types.h"
#include "toy_opcodes.h"

#include "toy_builtin.h"

#include <stdio.h>
#include <string.h>

static void printWrapper(const char* output) {
	//allow for disabling of newlines in the repl
#ifndef TOY_EXPORT
	if (Toy_commandLine.enablePrintNewline) {
		printf("%s\n", output);
	}
	else {
		printf("%s", output);
	}
#else
	printf("%s\n", output);
#endif
}

static void assertWrapper(const char* output) {
	fprintf(stderr, TOY_CC_ERROR "Assertion failure: %s\n" TOY_CC_RESET, output);
}

static void errorWrapper(const char* output) {
	fprintf(stderr, TOY_CC_ERROR "%s" TOY_CC_RESET, output); //no newline
}

bool Toy_injectNativeFn(Toy_Interpreter* interpreter, const char* name, Toy_NativeFn func) {
	//reject reserved words
	if (Toy_findTypeByKeyword(name) != TOY_TOKEN_EOF) {
		interpreter->errorOutput("Can't override an existing keyword\n");
		return false;
	}

	Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(name));

	//make sure the name isn't taken
	if (Toy_existsLiteralDictionary(&interpreter->scope->variables, identifier)) {
		interpreter->errorOutput("Can't override an existing variable\n");
		return false;
	}

	Toy_Literal fn = TOY_TO_FUNCTION_NATIVE_LITERAL(func);
	Toy_Literal type = TOY_TO_TYPE_LITERAL(fn.type, true);

	Toy_setLiteralDictionary(&interpreter->scope->variables, identifier, fn);
	Toy_setLiteralDictionary(&interpreter->scope->types, identifier, type);

	Toy_freeLiteral(identifier);
	Toy_freeLiteral(type);

	return true;
}

bool Toy_injectNativeHook(Toy_Interpreter* interpreter, const char* name, Toy_HookFn hook) {
	//reject reserved words
	if (Toy_findTypeByKeyword(name) != TOY_TOKEN_EOF) {
		interpreter->errorOutput("Can't inject a hook on an existing keyword\n");
		return false;
	}

	int identifierLength = strlen(name);
	Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(name, identifierLength));

	//make sure the name isn't taken
	if (Toy_existsLiteralDictionary(interpreter->hooks, identifier)) {
		interpreter->errorOutput("Can't override an existing hook\n");
		return false;
	}

	Toy_Literal fn = TOY_TO_FUNCTION_HOOK_LITERAL(hook);
	Toy_setLiteralDictionary(interpreter->hooks, identifier, fn);

	Toy_freeLiteral(identifier);

	return true;
}

void Toy_parseCompoundToPureValues(Toy_Interpreter* interpreter, Toy_Literal* literalPtr) {
	if (TOY_IS_IDENTIFIER(*literalPtr)) {
		Toy_parseIdentifierToValue(interpreter, literalPtr);
	}

	//parse out an array
	if (TOY_IS_ARRAY(*literalPtr)) {
		for (int i = 0; i < TOY_AS_ARRAY(*literalPtr)->count; i++) {
			Toy_Literal index = TOY_TO_INTEGER_LITERAL(i);
			Toy_Literal entry = Toy_getLiteralArray(TOY_AS_ARRAY(*literalPtr), index);

			if (TOY_IS_IDENTIFIER( entry )) {
				Toy_Literal idn = entry;
				Toy_parseCompoundToPureValues(interpreter, &entry);

				Toy_setLiteralArray(TOY_AS_ARRAY(*literalPtr), index, entry);

				Toy_freeLiteral(idn);
			}

			Toy_freeLiteral(index);
			Toy_freeLiteral(entry);
		}
	}

	//parse out a dictionary
	if (TOY_IS_DICTIONARY(*literalPtr)) {
		Toy_LiteralDictionary* ret = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(ret);

		for (int i = 0; i < TOY_AS_DICTIONARY(*literalPtr)->capacity; i++) {
			if ( TOY_IS_NULL(TOY_AS_DICTIONARY(*literalPtr)->entries[i].key) ) {
				continue;
			}

			Toy_Literal key = TOY_TO_NULL_LITERAL;
			Toy_Literal value = TOY_TO_NULL_LITERAL;

			key = Toy_copyLiteral(TOY_AS_DICTIONARY(*literalPtr)->entries[i].key);
			value = Toy_copyLiteral(TOY_AS_DICTIONARY(*literalPtr)->entries[i].value);

			//
			if (TOY_IS_IDENTIFIER( key ) || TOY_IS_IDENTIFIER(value)) {
				Toy_parseCompoundToPureValues(interpreter, &key);
				Toy_parseCompoundToPureValues(interpreter, &value);
			}

			Toy_setLiteralDictionary(ret, key, value);

			//
			Toy_freeLiteral(key);
			Toy_freeLiteral(value);
		}

		Toy_freeLiteral(*literalPtr);
		*literalPtr = TOY_TO_DICTIONARY_LITERAL(ret);
	}
}

bool Toy_parseIdentifierToValue(Toy_Interpreter* interpreter, Toy_Literal* literalPtr) {
	//this converts identifiers to values
	if (TOY_IS_IDENTIFIER(*literalPtr)) {
		if (!Toy_getScopeVariable(interpreter->scope, *literalPtr, literalPtr)) {
			interpreter->errorOutput("Undeclared variable ");
			Toy_printLiteralCustom(*literalPtr, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return false;
		}
	}

	if (TOY_IS_ARRAY(*literalPtr) || TOY_IS_DICTIONARY(*literalPtr)) {
		Toy_parseCompoundToPureValues(interpreter, literalPtr);
	}

	return true;
}

//utilities for the host program
void Toy_setInterpreterPrint(Toy_Interpreter* interpreter, Toy_PrintFn printOutput) {
	interpreter->printOutput = printOutput;
}

void Toy_setInterpreterAssert(Toy_Interpreter* interpreter, Toy_PrintFn assertOutput) {
	interpreter->assertOutput = assertOutput;
}

void Toy_setInterpreterError(Toy_Interpreter* interpreter, Toy_PrintFn errorOutput) {
	interpreter->errorOutput = errorOutput;
}

//utils
static unsigned char readByte(const unsigned char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	*count += 1;
	return ret;
}

static unsigned short readShort(const unsigned char* tb, int* count) {
	unsigned short ret = 0;
	memcpy(&ret, tb + *count, 2);
	*count += 2;
	return ret;
}

static int readInt(const unsigned char* tb, int* count) {
	int ret = 0;
	memcpy(&ret, tb + *count, 4);
	*count += 4;
	return ret;
}

static float readFloat(const unsigned char* tb, int* count) {
	float ret = 0;
	memcpy(&ret, tb + *count, 4);
	*count += 4;
	return ret;
}

static const char* readString(const unsigned char* tb, int* count) {
	const unsigned char* ret = tb + *count;
	*count += strlen((char*)ret) + 1; //+1 for null character
	return (const char*)ret;
}

static void consumeByte(Toy_Interpreter* interpreter, unsigned char byte, const unsigned char* tb, int* count) {
	if (byte != tb[*count]) {
		char buffer[512];
		snprintf(buffer, 512, "[internal] Failed to consume the correct byte (expected %u, found %u)\n", byte, tb[*count]);
		interpreter->errorOutput(buffer);
	}
	*count += 1;
}

static void consumeShort(Toy_Interpreter* interpreter, unsigned short bytes, const unsigned char* tb, int* count) {
	if (bytes != *(unsigned short*)(tb + *count)) {
		char buffer[512];
		snprintf(buffer, 512, "[internal] Failed to consume the correct bytes (expected %u, found %u)\n", bytes, *(unsigned short*)(tb + *count));
		interpreter->errorOutput(buffer);
	}
	*count += 2;
}

//each available statement
static bool execAssert(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	if (!TOY_IS_STRING(rhs)) {
		interpreter->errorOutput("The assert keyword needs a string as the second argument, received: ");
		Toy_printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(rhs);
		Toy_freeLiteral(lhs);
		return false;
	}

	if (TOY_IS_NULL(lhs) || !TOY_IS_TRUTHY(lhs)) {
		(*interpreter->assertOutput)(Toy_toCString(TOY_AS_STRING(rhs)));
		interpreter->panic = true;

		Toy_freeLiteral(rhs);
		Toy_freeLiteral(lhs);
		return false;
	}

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execPrint(Toy_Interpreter* interpreter) {
	//print what is on top of the stack, then pop it
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	if (TOY_IS_IDENTIFIER(lit)) {
		Toy_freeLiteral(lit);
		return false;
	}

	Toy_printLiteralCustom(lit, interpreter->printOutput);

	Toy_freeLiteral(lit);

	return true;
}

static bool execPushLiteral(Toy_Interpreter* interpreter, bool lng) {
	//read the index in the cache
	int index = 0;

	if (lng) {
		index = (int)readShort(interpreter->bytecode, &interpreter->count);
	}
	else {
		index = (int)readByte(interpreter->bytecode, &interpreter->count);
	}

	//push from cache to stack (DO NOT account for identifiers - will do that later)
	Toy_pushLiteralArray(&interpreter->stack, interpreter->literalCache.literals[index]);

	return true;
}

static bool rawLiteral(Toy_Interpreter* interpreter) {
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	if (TOY_IS_IDENTIFIER(lit)) {
		Toy_freeLiteral(lit);
		return false;
	}

	Toy_pushLiteralArray(&interpreter->stack, lit);
	Toy_freeLiteral(lit);

	return true;
}

static bool execNegate(Toy_Interpreter* interpreter) {
	//negate the top literal on the stack (numbers only)
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	if (TOY_IS_IDENTIFIER(lit)) {
		Toy_freeLiteral(lit);
		return false;
	}

	if (TOY_IS_INTEGER(lit)) {
		lit = TOY_TO_INTEGER_LITERAL(-TOY_AS_INTEGER(lit));
	}
	else if (TOY_IS_FLOAT(lit)) {
		lit = TOY_TO_FLOAT_LITERAL(-TOY_AS_FLOAT(lit));
	}
	else {
		interpreter->errorOutput("Can't negate that literal: ");
		Toy_printLiteralCustom(lit, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(lit);

		return false;
	}

	Toy_pushLiteralArray(&interpreter->stack, lit);
	Toy_freeLiteral(lit);

	return true;
}

static bool execInvert(Toy_Interpreter* interpreter) {
	//negate the top literal on the stack (booleans only)
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	if (TOY_IS_IDENTIFIER(lit)) {
		Toy_freeLiteral(lit);
		return false;
	}

	if (TOY_IS_BOOLEAN(lit)) {
		lit = TOY_TO_BOOLEAN_LITERAL(!TOY_AS_BOOLEAN(lit));
	}
	else {
		interpreter->errorOutput("Can't invert that literal: ");
		Toy_printLiteralCustom(lit, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(lit);

		return false;
	}

	Toy_pushLiteralArray(&interpreter->stack, lit);
	Toy_freeLiteral(lit);

	return true;
}

static bool execArithmetic(Toy_Interpreter* interpreter, Toy_Opcode opcode) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//special case for string concatenation ONLY
	if (TOY_IS_STRING(lhs) && TOY_IS_STRING(rhs) && (opcode == TOY_OP_ADDITION || opcode == TOY_OP_VAR_ADDITION_ASSIGN)) {
		//check for overflow
		int totalLength = TOY_AS_STRING(lhs)->length + TOY_AS_STRING(rhs)->length;
		if (totalLength > TOY_MAX_STRING_LENGTH) {
			interpreter->errorOutput("Can't concatenate these strings, result is too long (error found in interpreter)\n");
			return false;
		}

		//concat the strings
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, TOY_MAX_STRING_LENGTH, "%s%s", Toy_toCString(TOY_AS_STRING(lhs)), Toy_toCString(TOY_AS_STRING(rhs)));
		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, totalLength));
		Toy_pushLiteralArray(&interpreter->stack, literal);

		//cleanup
		Toy_freeLiteral(literal);
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return true;
	}

	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//maths based on types
	if(TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		switch(opcode) {
			case TOY_OP_ADDITION:
			case TOY_OP_VAR_ADDITION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) + TOY_AS_INTEGER(rhs) ));
				return true;

			case TOY_OP_SUBTRACTION:
			case TOY_OP_VAR_SUBTRACTION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) - TOY_AS_INTEGER(rhs) ));
				return true;

			case TOY_OP_MULTIPLICATION:
			case TOY_OP_VAR_MULTIPLICATION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) * TOY_AS_INTEGER(rhs) ));
				return true;

			case TOY_OP_DIVISION:
			case TOY_OP_VAR_DIVISION_ASSIGN:
				if (TOY_AS_INTEGER(rhs) == 0) {
					interpreter->errorOutput("Can't divide by zero (error found in interpreter)\n");
					return false;
				}
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) / TOY_AS_INTEGER(rhs) ));
				return true;

			case TOY_OP_MODULO:
			case TOY_OP_VAR_MODULO_ASSIGN:
				if (TOY_AS_INTEGER(rhs) == 0) {
					interpreter->errorOutput("Can't modulo by zero (error found in interpreter)\n");
					return false;
				}
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) % TOY_AS_INTEGER(rhs) ));
				return true;

			default:
				interpreter->errorOutput("[internal] bad opcode argument passed to execArithmetic()\n");
				return false;
		}
	}

	//catch bad modulo
	if (opcode == TOY_OP_MODULO || opcode == TOY_OP_VAR_MODULO_ASSIGN) {
		interpreter->errorOutput("Bad arithmetic argument (modulo on floats not allowed)\n");
		return false;
	}

	if(TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		switch(opcode) {
			case TOY_OP_ADDITION:
			case TOY_OP_VAR_ADDITION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) + TOY_AS_FLOAT(rhs) ));
				return true;

			case TOY_OP_SUBTRACTION:
			case TOY_OP_VAR_SUBTRACTION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) - TOY_AS_FLOAT(rhs) ));
				return true;

			case TOY_OP_MULTIPLICATION:
			case TOY_OP_VAR_MULTIPLICATION_ASSIGN:
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) * TOY_AS_FLOAT(rhs) ));
				return true;

			case TOY_OP_DIVISION:
			case TOY_OP_VAR_DIVISION_ASSIGN:
				if (TOY_AS_FLOAT(rhs) == 0) {
					interpreter->errorOutput("Can't divide by zero (error found in interpreter)\n");
					return false;
				}
				Toy_pushLiteralArray(&interpreter->stack, TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) / TOY_AS_FLOAT(rhs) ));
				return true;

			default:
				interpreter->errorOutput("[internal] bad opcode argument passed to execArithmetic()\n");
				return false;
		}
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return false;
}

static Toy_Literal parseTypeToValue(Toy_Interpreter* interpreter, Toy_Literal type) {
	//if an identifier is embedded in the type, figure out what it iss
	Toy_Literal typeIdn = type;
	if (TOY_IS_IDENTIFIER(type) && Toy_parseIdentifierToValue(interpreter, &type)) {
		Toy_freeLiteral(typeIdn);
	}

	if (TOY_IS_IDENTIFIER(type)) {
		return TOY_TO_NULL_LITERAL;
	}

	//if this is an array or dictionary, continue to the subtypes
	if (TOY_IS_TYPE(type) && (TOY_AS_TYPE(type).typeOf == TOY_LITERAL_ARRAY || TOY_AS_TYPE(type).typeOf == TOY_LITERAL_DICTIONARY)) {
		for (int i = 0; i < TOY_AS_TYPE(type).count; i++) {
			((Toy_Literal*)(TOY_AS_TYPE(type).subtypes))[i] = parseTypeToValue(interpreter, ((Toy_Literal*)(TOY_AS_TYPE(type).subtypes))[i]);
		}
	}

	//BUGFIX: make sure it actually is a type
	if (!TOY_IS_TYPE(type)) {
		interpreter->errorOutput("Bad type encountered: ");
		Toy_printLiteralCustom(type, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		//TODO: would be better to return an int here...
	}

	return type;
}

static bool execVarDecl(Toy_Interpreter* interpreter, bool lng) {
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

	Toy_Literal identifier = interpreter->literalCache.literals[identifierIndex];
	Toy_Literal type = Toy_copyLiteral(interpreter->literalCache.literals[typeIndex]);

	Toy_Literal typeIdn = type;
	if (TOY_IS_IDENTIFIER(type) && Toy_parseIdentifierToValue(interpreter, &type)) {
		Toy_freeLiteral(typeIdn);
	}

	if (TOY_IS_IDENTIFIER(type)) {
		Toy_freeLiteral(identifier);
		Toy_freeLiteral(type);
		return false;
	}

	//BUGFIX: because identifiers are getting embedded in type definitions
	type = parseTypeToValue(interpreter, type);

	if (!Toy_declareScopeVariable(interpreter->scope, identifier, type)) {
		interpreter->errorOutput("Can't redefine the variable \"");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	Toy_Literal val = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal valIdn = val;
	if (TOY_IS_IDENTIFIER(val) && Toy_parseIdentifierToValue(interpreter, &val)) {
		Toy_freeLiteral(valIdn);
	}

	if (TOY_IS_IDENTIFIER(val)) {
		Toy_freeLiteral(identifier);
		Toy_freeLiteral(type);
		Toy_freeLiteral(val);
		return false;
	}

	if (TOY_IS_ARRAY(val) || TOY_IS_DICTIONARY(val)) {
		Toy_parseCompoundToPureValues(interpreter, &val);
	}

	//TODO: could restrict opaque data to only opaque variables

	//BUGFIX: allow easy coercion on decl
	if (TOY_AS_TYPE(type).typeOf == TOY_LITERAL_FLOAT && TOY_IS_INTEGER(val)) {
		val = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(val));
	}

	if (!TOY_IS_NULL(val) && !Toy_setScopeVariable(interpreter->scope, identifier, val, false)) {
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(type);
		Toy_freeLiteral(val);

		return false;
	}

	Toy_freeLiteral(val);
	Toy_freeLiteral(type);

	return true;
}

static bool execFnDecl(Toy_Interpreter* interpreter, bool lng) {
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

	Toy_Literal identifier = interpreter->literalCache.literals[identifierIndex];
	Toy_Literal function = interpreter->literalCache.literals[functionIndex];

	TOY_AS_FUNCTION(function).scope = Toy_pushScope(interpreter->scope); //hacked in (needed for closure persistance)

	Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FUNCTION, true);

	if (!Toy_declareScopeVariable(interpreter->scope, identifier, type)) {
		interpreter->errorOutput("Can't redefine the function \"");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	if (!Toy_setScopeVariable(interpreter->scope, identifier, function, false)) { //scope gets copied here
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	Toy_popScope(TOY_AS_FUNCTION(function).scope); //hacked out
	TOY_AS_FUNCTION(function).scope = NULL;

	Toy_freeLiteral(type);

	return true;
}

static bool execVarAssign(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	if (TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	if (TOY_IS_ARRAY(rhs) || TOY_IS_DICTIONARY(rhs)) {
		Toy_parseCompoundToPureValues(interpreter, &rhs);
	}

	if (!TOY_IS_IDENTIFIER(lhs)) {
		interpreter->errorOutput("Can't assign to a non-variable \"");
		Toy_printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		return false;
	}

	if (!Toy_isDelcaredScopeVariable(interpreter->scope, lhs)) {
		interpreter->errorOutput("Undeclared variable \"");
		Toy_printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//BUGFIX: allow easy coercion on assign
	Toy_Literal type = Toy_getScopeType(interpreter->scope, lhs);
	if (TOY_AS_TYPE(type).typeOf == TOY_LITERAL_FLOAT && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (!Toy_setScopeVariable(interpreter->scope, lhs, rhs, true)) {
		interpreter->errorOutput("Incorrect type assigned to variable \"");
		Toy_printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		Toy_freeLiteral(type);
		return false;
	}

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);
	Toy_freeLiteral(type);

	return true;
}

static bool execVarArithmeticAssign(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	//duplicate the name
	Toy_pushLiteralArray(&interpreter->stack, lhs);
	Toy_pushLiteralArray(&interpreter->stack, lhs);
	Toy_pushLiteralArray(&interpreter->stack, rhs);

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execValCast(Toy_Interpreter* interpreter) {
	Toy_Literal value = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal type = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal valueIdn = value;
	if (TOY_IS_IDENTIFIER(value) && Toy_parseIdentifierToValue(interpreter, &value)) {
		Toy_freeLiteral(valueIdn);
	}

	if (TOY_IS_IDENTIFIER(value)) {
		Toy_freeLiteral(type);
		Toy_freeLiteral(value);
		return false;
	}

	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_NULL(value)) {
		interpreter->errorOutput("Can't cast a null value\n");

		Toy_freeLiteral(value);
		Toy_freeLiteral(type);

		return false;
	}

	//cast the rhs to the type represented by lhs
	switch(TOY_AS_TYPE(type).typeOf) {
		case TOY_LITERAL_BOOLEAN:
			result = TOY_TO_BOOLEAN_LITERAL(TOY_IS_TRUTHY(value));
		break;

		case TOY_LITERAL_INTEGER:
			if (TOY_IS_BOOLEAN(value)) {
				result = TOY_TO_INTEGER_LITERAL(TOY_AS_BOOLEAN(value) ? 1 : 0);
			}

			if (TOY_IS_INTEGER(value)) {
				result = Toy_copyLiteral(value);
			}

			if (TOY_IS_FLOAT(value)) {
				result = TOY_TO_INTEGER_LITERAL(TOY_AS_FLOAT(value));
			}

			if (TOY_IS_STRING(value)) {
				int val = 0;
				sscanf(Toy_toCString(TOY_AS_STRING(value)), "%d", &val);
				result = TOY_TO_INTEGER_LITERAL(val);
			}
		break;

		case TOY_LITERAL_FLOAT:
			if (TOY_IS_BOOLEAN(value)) {
				result = TOY_TO_FLOAT_LITERAL(TOY_AS_BOOLEAN(value) ? 1 : 0);
			}

			if (TOY_IS_INTEGER(value)) {
				result = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(value));
			}

			if (TOY_IS_FLOAT(value)) {
				result = Toy_copyLiteral(value);
			}

			if (TOY_IS_STRING(value)) {
				float val = 0;
				sscanf(Toy_toCString(TOY_AS_STRING(value)), "%f", &val);
				result = TOY_TO_FLOAT_LITERAL(val);
			}
		break;

		case TOY_LITERAL_STRING:
			if (TOY_IS_BOOLEAN(value)) {
				char* str = TOY_AS_BOOLEAN(value) ? "true" : "false";

				int length = strlen(str);
				result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(str, length)); //TODO: static reference optimisation?
			}

			if (TOY_IS_INTEGER(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%d", TOY_AS_INTEGER(value));
				int length = strlen(buffer);
				result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, length));
			}

			if (TOY_IS_FLOAT(value)) {
				char buffer[128];
				snprintf(buffer, 128, "%g", TOY_AS_FLOAT(value));
				int length = strlen(buffer);
				result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, length));
			}

			if (TOY_IS_STRING(value)) {
				result = Toy_copyLiteral(value);
			}
		break;

		default:
			interpreter->errorOutput("Unknown cast type found: ");
			Toy_printLiteralCustom(type, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return false;
	}

	//leave the new value on the stack
	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(value);
	Toy_freeLiteral(type);

	return true;
}

static bool execTypeOf(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal type = TOY_TO_NULL_LITERAL;

	if (TOY_IS_IDENTIFIER(rhs)) {
		type = Toy_getScopeType(interpreter->scope, rhs);
	}
	else {
		type = TOY_TO_TYPE_LITERAL(rhs.type, false); //see issue #53
	}

	Toy_pushLiteralArray(&interpreter->stack, type);

	Toy_freeLiteral(rhs);
	Toy_freeLiteral(type);

	return true;
}

static bool execCompareEqual(Toy_Interpreter* interpreter, bool invert) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	bool result = Toy_literalsAreEqual(lhs, rhs);

	if (invert) {
		result = !result;
	}

	Toy_pushLiteralArray(&interpreter->stack, TOY_TO_BOOLEAN_LITERAL(result));

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execCompareLess(Toy_Interpreter* interpreter, bool invert) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//not a number, return falure
	if (!(TOY_IS_INTEGER(lhs) || TOY_IS_FLOAT(lhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		Toy_printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	if (!(TOY_IS_INTEGER(rhs) || TOY_IS_FLOAT(rhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		Toy_printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//convert to floats - easier
	if (TOY_IS_INTEGER(lhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	if (TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	bool result;

	if (!invert) {
		result = (TOY_AS_FLOAT(lhs) < TOY_AS_FLOAT(rhs));
	}
	else {
		result = (TOY_AS_FLOAT(lhs) > TOY_AS_FLOAT(rhs));
	}

	Toy_pushLiteralArray(&interpreter->stack, TOY_TO_BOOLEAN_LITERAL(result));

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execCompareLessEqual(Toy_Interpreter* interpreter, bool invert) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//not a number, return falure
	if (!(TOY_IS_INTEGER(lhs) || TOY_IS_FLOAT(lhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		Toy_printLiteralCustom(lhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	if (!(TOY_IS_INTEGER(rhs) || TOY_IS_FLOAT(rhs))) {
		interpreter->errorOutput("Incorrect type in comparison, value \"");
		Toy_printLiteralCustom(rhs, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//convert to floats - easier
	if (TOY_IS_INTEGER(lhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	if (TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	bool result;

	if (!invert) {
		result = (TOY_AS_FLOAT(lhs) < TOY_AS_FLOAT(rhs)) || Toy_literalsAreEqual(lhs, rhs);
	}
	else {
		result = (TOY_AS_FLOAT(lhs) > TOY_AS_FLOAT(rhs)) || Toy_literalsAreEqual(lhs, rhs);
	}

	Toy_pushLiteralArray(&interpreter->stack, TOY_TO_BOOLEAN_LITERAL(result));

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execAnd(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//short-circuit support
	if (!TOY_IS_TRUTHY(lhs)) {
		Toy_pushLiteralArray(&interpreter->stack, lhs);
	}
	else {
		Toy_pushLiteralArray(&interpreter->stack, rhs);
	}

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execOr(Toy_Interpreter* interpreter) {
	Toy_Literal rhs = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal lhs = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal rhsIdn = rhs;
	if (TOY_IS_IDENTIFIER(rhs) && Toy_parseIdentifierToValue(interpreter, &rhs)) {
		Toy_freeLiteral(rhsIdn);
	}

	Toy_Literal lhsIdn = lhs;
	if (TOY_IS_IDENTIFIER(lhs) && Toy_parseIdentifierToValue(interpreter, &lhs)) {
		Toy_freeLiteral(lhsIdn);
	}

	if (TOY_IS_IDENTIFIER(lhs) || TOY_IS_IDENTIFIER(rhs)) {
		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);
		return false;
	}

	//short-circuit support
	if (TOY_IS_TRUTHY(lhs)) {
		Toy_pushLiteralArray(&interpreter->stack, lhs);
	}
	else {
		Toy_pushLiteralArray(&interpreter->stack, rhs);
	}

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return true;
}

static bool execJump(Toy_Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		interpreter->errorOutput("[internal] Jump out of range\n");
		return false;
	}

	//actually jump
	interpreter->count = target + interpreter->codeStart;

	return true;
}

static bool execFalseJump(Toy_Interpreter* interpreter) {
	int target = (int)readShort(interpreter->bytecode, &interpreter->count);

	if (target + interpreter->codeStart > interpreter->length) {
		interpreter->errorOutput("[internal] Jump out of range (false jump)\n");
		return false;
	}

	//actually jump
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal litIdn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(litIdn);
	}

	if (TOY_IS_IDENTIFIER(lit)) {
		Toy_freeLiteral(lit);
		return false;
	}

	if (TOY_IS_NULL(lit)) {
		interpreter->errorOutput("Null detected in comparison\n");
		Toy_freeLiteral(lit);
		return false;
	}

	if (!TOY_IS_TRUTHY(lit)) {
		interpreter->count = target + interpreter->codeStart;
	}

	Toy_freeLiteral(lit);

	return true;
}

//forward declare
static void execInterpreter(Toy_Interpreter*);
static void readInterpreterSections(Toy_Interpreter* interpreter);

//expect stack: identifier, arg1, arg2, arg3..., stackSize
//also supports identifier & arg1 to be other way around (looseFirstArgument)
static bool execFnCall(Toy_Interpreter* interpreter, bool looseFirstArgument) {
	//BUGFIX: depth check - don't drown!
	if (interpreter->depth >= 1000) {
		interpreter->errorOutput("Infinite recursion detected - panicking\n");
		interpreter->panic = true;
		return false;
	}

	Toy_LiteralArray arguments;
	Toy_initLiteralArray(&arguments);

	Toy_Literal stackSize = Toy_popLiteralArray(&interpreter->stack);

	//unpack the stack of arguments
	for (int i = 0; i < TOY_AS_INTEGER(stackSize) - 1; i++) {
		Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);
		Toy_pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		Toy_freeLiteral(lit);
	}

	//collect one more argument
	if (!looseFirstArgument && TOY_AS_INTEGER(stackSize) > 0) {
		Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);
		Toy_pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		Toy_freeLiteral(lit);
	}

	Toy_Literal identifier = Toy_popLiteralArray(&interpreter->stack);

	//collect one more argument
	if (looseFirstArgument) {
		Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);
		Toy_pushLiteralArray(&arguments, lit); //NOTE: also reverses the order
		Toy_freeLiteral(lit);
	}

	//get the function literal
	Toy_Literal func = identifier;

	if (!Toy_parseIdentifierToValue(interpreter, &func)) {
		Toy_freeLiteralArray(&arguments);
		Toy_freeLiteral(stackSize);
		Toy_freeLiteral(identifier);
		return false;
	}

	if (!TOY_IS_FUNCTION(func) && !TOY_IS_FUNCTION_NATIVE(func)) {
		interpreter->errorOutput("Function not found: ");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(identifier);
		Toy_freeLiteral(stackSize);
		Toy_freeLiteralArray(&arguments);
		return false;
	}

	//BUGFIX: correct the argument order
	Toy_LiteralArray correct;
	Toy_initLiteralArray(&correct);

	while (arguments.count > 0) {
		Toy_Literal lit = Toy_popLiteralArray(&arguments);
		Toy_pushLiteralArray(&correct, lit);
		Toy_freeLiteral(lit);
	}

	//call the function literal
	bool ret = Toy_callLiteralFn(interpreter, func, &correct, &interpreter->stack);

	if (!ret) {
		interpreter->errorOutput("Error encountered in function \"");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");
	}

	Toy_freeLiteralArray(&correct);
	Toy_freeLiteralArray(&arguments);
	Toy_freeLiteral(func);
	Toy_freeLiteral(stackSize);
	Toy_freeLiteral(identifier);

	return ret;
}

//expects arguments in correct order
bool Toy_callLiteralFn(Toy_Interpreter* interpreter, Toy_Literal func, Toy_LiteralArray* arguments, Toy_LiteralArray* returns) {
	//check for side-loaded native functions
	if (TOY_IS_FUNCTION_NATIVE(func)) {
		//TODO: parse out identifier values, see issue #64

		//call the native function
		int returnsCount = TOY_AS_FUNCTION_NATIVE(func)(interpreter, arguments);

		if (returnsCount < 0) {
			// interpreter->errorOutput("Unknown error from native function\n");
			return false;
		}

		//get the results
		Toy_LiteralArray returnsFromInner;
		Toy_initLiteralArray(&returnsFromInner);

		for (int i = 0; i < (returnsCount || 1); i++) {
			Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);
			Toy_pushLiteralArray(&returnsFromInner, lit); //NOTE: also reverses the order
			Toy_freeLiteral(lit);
		}

		//flip them around and pass to returns
		while (returnsFromInner.count > 0) {
			Toy_Literal lit = Toy_popLiteralArray(&returnsFromInner);
			Toy_pushLiteralArray(returns, lit);
			Toy_freeLiteral(lit);
		}

		Toy_freeLiteralArray(&returnsFromInner);
		return true;
	}

	//normal Toy function
	if (!TOY_IS_FUNCTION(func)) {
		interpreter->errorOutput("Function literal required in Toy_callLiteralFn()\n");
		return false;
	}

	//set up a new interpreter
	Toy_Interpreter inner;

	//init the inner interpreter manually
	Toy_initLiteralArray(&inner.literalCache);
	inner.scope = Toy_pushScope(func.as.function.scope);
	inner.bytecode = ((Toy_RefFunction*)(TOY_AS_FUNCTION(func).inner.ptr))->data;
	inner.length = ((Toy_RefFunction*)(TOY_AS_FUNCTION(func).inner.ptr))->length;
	inner.count = 0;
	inner.codeStart = -1;
	inner.depth = interpreter->depth + 1;
	inner.panic = false;
	Toy_initLiteralArray(&inner.stack);
	inner.hooks = interpreter->hooks;
	Toy_setInterpreterPrint(&inner, interpreter->printOutput);
	Toy_setInterpreterAssert(&inner, interpreter->assertOutput);
	Toy_setInterpreterError(&inner, interpreter->errorOutput);

	//prep the sections
	readInterpreterSections(&inner);

	//prep the arguments
	Toy_LiteralArray* paramArray = TOY_AS_ARRAY(inner.literalCache.literals[ readShort(inner.bytecode, &inner.count) ]);
	Toy_LiteralArray* returnArray = TOY_AS_ARRAY(inner.literalCache.literals[ readShort(inner.bytecode, &inner.count) ]);

	//get the rest param, if it exists
	Toy_Literal restParam = TOY_TO_NULL_LITERAL;
	if (paramArray->count >= 2 && TOY_AS_TYPE(paramArray->literals[ paramArray->count -1 ]).typeOf == TOY_LITERAL_FUNCTION_ARG_REST) {
		restParam = paramArray->literals[ paramArray->count -2 ];
	}

	//check the param total is correct
	if ((TOY_IS_NULL(restParam) && paramArray->count != arguments->count * 2) || (!TOY_IS_NULL(restParam) && paramArray->count -2 > arguments->count * 2)) {
		interpreter->errorOutput("Incorrect number of arguments passed to a function\n");

		//free, and skip out
		Toy_popScope(inner.scope);

		Toy_freeLiteralArray(&inner.stack);
		Toy_freeLiteralArray(&inner.literalCache);

		return false;
	}

	//BUGFIX: access the arguments from the beginning
	int argumentIndex = 0;

	//contents is the indexes of identifier & type
	for (int i = 0; i < paramArray->count - (TOY_IS_NULL(restParam) ? 0 : 2); i += 2) { //don't count the rest parameter, if present
		//declare and define each entry in the scope
		if (!Toy_declareScopeVariable(inner.scope, paramArray->literals[i], paramArray->literals[i + 1])) {
			interpreter->errorOutput("[internal] Could not re-declare parameter\n");

			//free, and skip out
			Toy_popScope(inner.scope);

			Toy_freeLiteralArray(&inner.stack);
			Toy_freeLiteralArray(&inner.literalCache);

			return false;
		}

		//access the arguments in order
		Toy_Literal arg = TOY_TO_NULL_LITERAL;
		if (argumentIndex < arguments->count) {
			arg = Toy_copyLiteral(arguments->literals[argumentIndex++]);
		}

		Toy_Literal argIdn = arg;
		if (TOY_IS_IDENTIFIER(arg) && Toy_parseIdentifierToValue(interpreter, &arg)) {
			Toy_freeLiteral(argIdn);
		}

		if (TOY_IS_IDENTIFIER(arg)) {
			//free, and skip out
			Toy_freeLiteral(arg);
			Toy_popScope(inner.scope);

			Toy_freeLiteralArray(&inner.stack);
			Toy_freeLiteralArray(&inner.literalCache);

			return false;
		}

		if (!Toy_setScopeVariable(inner.scope, paramArray->literals[i], arg, false)) {
			interpreter->errorOutput("[internal] Could not define parameter (bad type?)\n");

			//free, and skip out
			Toy_freeLiteral(arg);
			Toy_popScope(inner.scope);

			Toy_freeLiteralArray(&inner.stack);
			Toy_freeLiteralArray(&inner.literalCache);

			return false;
		}
		Toy_freeLiteral(arg);
	}

	//if using rest, pack the optional extra arguments into the rest parameter (array)
	if (!TOY_IS_NULL(restParam)) {
		Toy_LiteralArray rest;
		Toy_initLiteralArray(&rest);

		//access the arguments in order
		while (argumentIndex < arguments->count) {
			Toy_Literal lit = Toy_copyLiteral(arguments->literals[argumentIndex++]);
			Toy_pushLiteralArray(&rest, lit);
			Toy_freeLiteral(lit);
		}

		Toy_Literal restType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ARRAY, true);
		Toy_Literal any = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, false);
		TOY_TYPE_PUSH_SUBTYPE(&restType, any);

		//declare & define the rest parameter
		if (!Toy_declareScopeVariable(inner.scope, restParam, restType)) {
			interpreter->errorOutput("[internal] Could not declare rest parameter\n");

			//free, and skip out
			Toy_freeLiteral(restType);
			Toy_freeLiteralArray(&rest);
			Toy_popScope(inner.scope);

			Toy_freeLiteralArray(&inner.stack);
			Toy_freeLiteralArray(&inner.literalCache);

			return false;
		}

		Toy_Literal lit = TOY_TO_ARRAY_LITERAL(&rest);
		if (!Toy_setScopeVariable(inner.scope, restParam, lit, false)) {
			interpreter->errorOutput("[internal] Could not define rest parameter\n");

			//free, and skip out
			Toy_freeLiteral(restType);
			Toy_freeLiteral(lit);
			Toy_popScope(inner.scope);

			Toy_freeLiteralArray(&inner.stack);
			Toy_freeLiteralArray(&inner.literalCache);

			return false;
		}

		Toy_freeLiteral(restType);
		Toy_freeLiteralArray(&rest);
	}

	//execute the interpreter
	execInterpreter(&inner);

	//adopt the panic state
	interpreter->panic = inner.panic;

	//accept the stack as the results
	Toy_LiteralArray returnsFromInner;
	Toy_initLiteralArray(&returnsFromInner);

	//unpack the results
	for (int i = 0; i < (returnArray->count || 1); i++) {
		Toy_Literal lit = Toy_popLiteralArray(&inner.stack);
		Toy_pushLiteralArray(&returnsFromInner, lit); //NOTE: also reverses the order
		Toy_freeLiteral(lit);
	}

	bool returnValue = true;

	//TODO: remove this when multiple assignment is enabled - note the BUGFIX that balances the stack
	if (returnsFromInner.count > 1) {
		interpreter->errorOutput("Too many values returned (multiple returns not yet supported)\n");

		returnValue = false;
	}

	for (int i = 0; i < returnsFromInner.count && returnValue; i++) {
		Toy_Literal ret = Toy_popLiteralArray(&returnsFromInner);

		//check the return types
		if (returnArray->count > 0 && TOY_AS_TYPE(returnArray->literals[i]).typeOf != ret.type) {
			interpreter->errorOutput("Bad type found in return value\n");

			//free, and skip out
			returnValue = false;
			break;
		}

		Toy_pushLiteralArray(returns, ret); //NOTE: reverses again
		Toy_freeLiteral(ret);
	}

	//manual free
	//BUGFIX: handle scopes of functions, which refer to the parent scope (leaking memory)
	while(inner.scope != TOY_AS_FUNCTION(func).scope) {
		for (int i = 0; i < inner.scope->variables.capacity; i++) {
			//handle keys, just in case
			if (TOY_IS_FUNCTION(inner.scope->variables.entries[i].key)) {
				Toy_popScope(TOY_AS_FUNCTION(inner.scope->variables.entries[i].key).scope);
				TOY_AS_FUNCTION(inner.scope->variables.entries[i].key).scope = NULL;
			}

			if (TOY_IS_FUNCTION(inner.scope->variables.entries[i].value)) {
				Toy_popScope(TOY_AS_FUNCTION(inner.scope->variables.entries[i].value).scope);
				TOY_AS_FUNCTION(inner.scope->variables.entries[i].value).scope = NULL;
			}
		}

		inner.scope = Toy_popScope(inner.scope);
	}
	Toy_freeLiteralArray(&returnsFromInner);
	Toy_freeLiteralArray(&inner.stack);
	Toy_freeLiteralArray(&inner.literalCache);

	//BUGFIX: this function needs to eat the arguments
	Toy_freeLiteralArray(arguments);

	//actual bytecode persists until next call
	return true;
}

bool Toy_callFn(Toy_Interpreter* interpreter, const char* name, Toy_LiteralArray* arguments, Toy_LiteralArray* returns) {
	Toy_Literal key = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(name, strlen(name)));
	Toy_Literal val = TOY_TO_NULL_LITERAL;
	
	if (!Toy_isDelcaredScopeVariable(interpreter->scope, key)) {
		interpreter->errorOutput("No function with that name\n");
		return false;
	}

	Toy_getScopeVariable(interpreter->scope, key, &val);

	bool ret = Toy_callLiteralFn(interpreter, val, arguments, returns);

	Toy_freeLiteral(key);
	Toy_freeLiteral(val);

	return ret;
}

static bool execFnReturn(Toy_Interpreter* interpreter) {
	Toy_LiteralArray returns;
	Toy_initLiteralArray(&returns);

	//get the values of everything on the stack
	while (interpreter->stack.count > 0) {
		Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

		Toy_Literal litIdn = lit;
		if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
			Toy_freeLiteral(litIdn);
		}

		if (TOY_IS_IDENTIFIER(lit)) {
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(lit);
			return false;
		}

		if (TOY_IS_ARRAY(lit) || TOY_IS_DICTIONARY(lit)) {
			Toy_parseCompoundToPureValues(interpreter, &lit);
		}

		Toy_pushLiteralArray(&returns, lit); //reverses the order
		Toy_freeLiteral(lit);
	}

	//and back again
	while (returns.count > 0) {
		Toy_Literal lit =  Toy_popLiteralArray(&returns);
		Toy_pushLiteralArray(&interpreter->stack, lit);
		Toy_freeLiteral(lit);
	}

	Toy_freeLiteralArray(&returns);

	//finally
	return false;
}

static bool execImport(Toy_Interpreter* interpreter) {
	Toy_Literal alias = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal identifier = Toy_popLiteralArray(&interpreter->stack);

	//access the hooks
	if (!Toy_existsLiteralDictionary(interpreter->hooks, identifier)) {
		interpreter->errorOutput("Unknown library name in import statement: ");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(alias);
		Toy_freeLiteral(identifier);
		return false;
	}

	Toy_Literal func = Toy_getLiteralDictionary(interpreter->hooks, identifier);

	if (!TOY_IS_FUNCTION_HOOK(func)) {
		interpreter->errorOutput("Expected hook function, found: ");
		Toy_printLiteralCustom(identifier, interpreter->errorOutput);
		interpreter->errorOutput("\"\n");

		Toy_freeLiteral(func);
		Toy_freeLiteral(alias);
		Toy_freeLiteral(identifier);
		return false;
	}

	TOY_AS_FUNCTION_HOOK(func)(interpreter, identifier, alias);

	Toy_freeLiteral(func);
	Toy_freeLiteral(alias);
	Toy_freeLiteral(identifier);
	return true;
}

static bool execIndex(Toy_Interpreter* interpreter, bool assignIntermediate) {
	//assume -> compound, first, second, third are all on the stack

	Toy_Literal third = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal second = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal first = Toy_popLiteralArray(&interpreter->stack);
	Toy_Literal compound = Toy_popLiteralArray(&interpreter->stack);

	Toy_Literal compoundIdn = compound;
	bool freeIdn = false;
	if (TOY_IS_IDENTIFIER(compound) && Toy_parseIdentifierToValue(interpreter, &compound)) {
		freeIdn = true;
	}

	if (TOY_IS_IDENTIFIER(compound)) {
		Toy_freeLiteral(third);
		Toy_freeLiteral(second);
		Toy_freeLiteral(first);
		Toy_freeLiteral(compound);
		if (freeIdn) {
			Toy_freeLiteral(compoundIdn);
		}
		return true;
	}

	if (!TOY_IS_ARRAY(compound) && !TOY_IS_DICTIONARY(compound) && !TOY_IS_STRING(compound)) {
		interpreter->errorOutput("Unknown compound found in index notation: ");
		Toy_printLiteralCustom(compound, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		Toy_freeLiteral(third);
		Toy_freeLiteral(second);
		Toy_freeLiteral(first);
		Toy_freeLiteral(compound);

		if (freeIdn) {
			Toy_freeLiteral(compoundIdn);
		}

		return false;
	}

	//build the argument list
	Toy_LiteralArray arguments;
	Toy_initLiteralArray(&arguments);

	Toy_pushLiteralArray(&arguments, compound);
	Toy_pushLiteralArray(&arguments, first);
	Toy_pushLiteralArray(&arguments, second);
	Toy_pushLiteralArray(&arguments, third);
	Toy_pushLiteralArray(&arguments, TOY_TO_NULL_LITERAL); //it expects an assignment command
	Toy_pushLiteralArray(&arguments, TOY_TO_NULL_LITERAL); //it expects an assignment "opcode"

	//leave the idn and compound on the stack
	if (assignIntermediate) {
		if (TOY_IS_IDENTIFIER(compoundIdn)) {
			Toy_pushLiteralArray(&interpreter->stack, compoundIdn);
		}
		Toy_pushLiteralArray(&interpreter->stack, compound);
		Toy_pushLiteralArray(&interpreter->stack, first);
		Toy_pushLiteralArray(&interpreter->stack, second);
		Toy_pushLiteralArray(&interpreter->stack, third);
	}

	//call the index function
	if (Toy_private_index(interpreter, &arguments) < 0) {
		interpreter->errorOutput("Something went wrong while indexing (simple index): ");
		Toy_printLiteralCustom(compoundIdn, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		//clean up
		Toy_freeLiteral(third);
		Toy_freeLiteral(second);
		Toy_freeLiteral(first);
		Toy_freeLiteral(compound);
		if (freeIdn) {
			Toy_freeLiteral(compoundIdn);
		}
		Toy_freeLiteralArray(&arguments);
		return false;
	}

	//clean up
	Toy_freeLiteral(third);
	Toy_freeLiteral(second);
	Toy_freeLiteral(first);
	Toy_freeLiteral(compound);
	if (freeIdn) {
		Toy_freeLiteral(compoundIdn);
	}
	Toy_freeLiteralArray(&arguments);

	return true;
}

static bool execIndexAssign(Toy_Interpreter* interpreter, int assignDepth) {
	//assume -> compound, first, second, third, assign are all on the stack

	Toy_Literal assign = TOY_TO_NULL_LITERAL, third = TOY_TO_NULL_LITERAL, second = TOY_TO_NULL_LITERAL, first = TOY_TO_NULL_LITERAL, compound = TOY_TO_NULL_LITERAL, result = TOY_TO_NULL_LITERAL;
	Toy_Literal compoundIdn = TOY_TO_NULL_LITERAL;
	bool freeIdn = false;

	//build the opcode
	unsigned char opcode = readByte(interpreter->bytecode, &interpreter->count);
	char* opStr = "";
	switch (opcode) {
	case TOY_OP_VAR_ASSIGN:
		opStr = "=";
		break;
	case TOY_OP_VAR_ADDITION_ASSIGN:
		opStr = "+=";
		break;
	case TOY_OP_VAR_SUBTRACTION_ASSIGN:
		opStr = "-=";
		break;
	case TOY_OP_VAR_MULTIPLICATION_ASSIGN:
		opStr = "*=";
		break;
	case TOY_OP_VAR_DIVISION_ASSIGN:
		opStr = "/=";
		break;
	case TOY_OP_VAR_MODULO_ASSIGN:
		opStr = "%=";
		break;

	default:
		interpreter->errorOutput("bad opcode in index assigning notation\n");
		return false;
	}

	//iterate...
	while(assignDepth-- >= 0) {
		Toy_freeLiteral(assign);
		Toy_freeLiteral(third);
		Toy_freeLiteral(second);
		Toy_freeLiteral(first);
		Toy_freeLiteral(compound);

		if (TOY_IS_NULL(result)) {
			assign = Toy_popLiteralArray(&interpreter->stack);
		}
		else {
			assign = result;
		}

		third = Toy_popLiteralArray(&interpreter->stack);
		second = Toy_popLiteralArray(&interpreter->stack);
		first = Toy_popLiteralArray(&interpreter->stack);
		compound = Toy_popLiteralArray(&interpreter->stack);

		if (TOY_IS_IDENTIFIER(compound)) {
			if (freeIdn) {
				Toy_freeLiteral(compoundIdn);
			}

			compoundIdn = compound;
			Toy_parseIdentifierToValue(interpreter, &compound);
			freeIdn = true;
		}

		if (TOY_IS_IDENTIFIER(compound)) {
			Toy_freeLiteral(compound);
			Toy_freeLiteral(first);
			Toy_freeLiteral(second);
			Toy_freeLiteral(third);
			Toy_freeLiteral(assign);
			if (freeIdn) {
				Toy_freeLiteral(compoundIdn);
			}
			return false;
		}

		Toy_Literal assignIdn = assign;
		if (TOY_IS_IDENTIFIER(assign) && Toy_parseIdentifierToValue(interpreter, &assign)) {
			Toy_freeLiteral(assignIdn);
		}

		if (TOY_IS_IDENTIFIER(assign)) {
			Toy_freeLiteral(compound);
			Toy_freeLiteral(first);
			Toy_freeLiteral(second);
			Toy_freeLiteral(third);
			Toy_freeLiteral(assign);
			return false;
		}

		if (!TOY_IS_ARRAY(compound) && !TOY_IS_DICTIONARY(compound) && !TOY_IS_STRING(compound)) {
			interpreter->errorOutput("Unknown compound found in index assigning notation: ");
			Toy_printLiteralCustom(compound, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			Toy_freeLiteral(assign);
			Toy_freeLiteral(third);
			Toy_freeLiteral(second);
			Toy_freeLiteral(first);
			Toy_freeLiteral(compound);
			if (freeIdn) {
				Toy_freeLiteral(compoundIdn);
			}
			return false;
		}

		int opLength = strlen(opStr);
		Toy_Literal op = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(opStr, opLength)); //TODO: static reference optimisation?

		//build the argument list
		Toy_LiteralArray arguments;
		Toy_initLiteralArray(&arguments);

		Toy_pushLiteralArray(&arguments, compound);
		Toy_pushLiteralArray(&arguments, first);
		Toy_pushLiteralArray(&arguments, second);
		Toy_pushLiteralArray(&arguments, third);
		Toy_pushLiteralArray(&arguments, assign); //it expects an assignment command
		Toy_pushLiteralArray(&arguments, op); //it expects an assignment "opcode"

		//call the index function
		if (Toy_private_index(interpreter, &arguments) < 0) {
			//clean up
			Toy_freeLiteral(assign);
			Toy_freeLiteral(third);
			Toy_freeLiteral(second);
			Toy_freeLiteral(first);
			Toy_freeLiteral(compound);
			if (freeIdn) {
				Toy_freeLiteral(compoundIdn);
			}
			Toy_freeLiteral(op);
			Toy_freeLiteralArray(&arguments);

			return false;
		}

		//save the result (assume top of the interpreter stack is the new compound value)
		result = Toy_popLiteralArray(&interpreter->stack);

		Toy_freeLiteral(op);
		Toy_freeLiteralArray(&arguments);

		//if we loop, then we need to be assigning
		opStr = "=";
	}

	//BUGFIX: make sure the compound name can be assigned
	if (TOY_IS_NULL(compoundIdn)) {
		compoundIdn = Toy_popLiteralArray(&interpreter->stack);
		freeIdn = true;
	}

	if (TOY_IS_IDENTIFIER(compoundIdn) && !Toy_setScopeVariable(interpreter->scope, compoundIdn, result, true)) {
		interpreter->errorOutput("Incorrect type assigned to compound member ");
		Toy_printLiteralCustom(compoundIdn, interpreter->errorOutput);
		interpreter->errorOutput(", value: ");
		Toy_printLiteralCustom(result, interpreter->errorOutput);
		interpreter->errorOutput("\n");

		//clean up
		Toy_freeLiteral(assign);
		Toy_freeLiteral(third);
		Toy_freeLiteral(second);
		Toy_freeLiteral(first);
		Toy_freeLiteral(compound);
		if (freeIdn) {
			Toy_freeLiteral(compoundIdn);
		}
		Toy_freeLiteral(result);
		return false;
	}

	//clean up
	Toy_freeLiteral(assign);
	Toy_freeLiteral(third);
	Toy_freeLiteral(second);
	Toy_freeLiteral(first);
	Toy_freeLiteral(compound);
	if (freeIdn) {
		Toy_freeLiteral(compoundIdn);
	}
	Toy_freeLiteral(result);

	return true;
}

//the heart of toy
static void execInterpreter(Toy_Interpreter* interpreter) {
	//set the starting point for the interpreter
	if (interpreter->codeStart == -1) {
		interpreter->codeStart = interpreter->count;
	}

	//BUGFIX
	int intermediateAssignDepth = 0;

	unsigned char opcode = readByte(interpreter->bytecode, &interpreter->count);

	while(opcode != TOY_OP_EOF && opcode != TOY_OP_SECTION_END && !interpreter->panic) {
		switch(opcode) {
			case TOY_OP_PASS:
				//DO NOTHING
			break;

			case TOY_OP_ASSERT:
				if (!execAssert(interpreter)) {
					return;
				}
			break;

			case TOY_OP_PRINT:
				if (!execPrint(interpreter)) {
					return;
				}
			break;

			case TOY_OP_LITERAL:
			case TOY_OP_LITERAL_LONG:
				if (!execPushLiteral(interpreter, opcode == TOY_OP_LITERAL_LONG)) {
					return;
				}
			break;

			case TOY_OP_LITERAL_RAW:
				if (!rawLiteral(interpreter)) {
					return;
				}
			break;

			case TOY_OP_NEGATE:
				if (!execNegate(interpreter)) {
					return;
				}
			break;

			case TOY_OP_ADDITION:
			case TOY_OP_SUBTRACTION:
			case TOY_OP_MULTIPLICATION:
			case TOY_OP_DIVISION:
			case TOY_OP_MODULO:
				if (!execArithmetic(interpreter, opcode)) {
					return;
				}
			break;

			case TOY_OP_VAR_ADDITION_ASSIGN:
			case TOY_OP_VAR_SUBTRACTION_ASSIGN:
			case TOY_OP_VAR_MULTIPLICATION_ASSIGN:
			case TOY_OP_VAR_DIVISION_ASSIGN:
			case TOY_OP_VAR_MODULO_ASSIGN:
				execVarArithmeticAssign(interpreter);
				if (!execArithmetic(interpreter, opcode)) {
					Toy_freeLiteral(Toy_popLiteralArray(&interpreter->stack));
					return;
				}
				if (!execVarAssign(interpreter)) {
					return;
				}
			break;

			case TOY_OP_GROUPING_BEGIN:
				execInterpreter(interpreter);
			break;

			case TOY_OP_GROUPING_END:
				return;

			//scope
			case TOY_OP_SCOPE_BEGIN:
				interpreter->scope = Toy_pushScope(interpreter->scope);
			break;

			case TOY_OP_SCOPE_END:
				interpreter->scope = Toy_popScope(interpreter->scope);
			break;

			//TODO: custom type declarations?

			case TOY_OP_VAR_DECL:
			case TOY_OP_VAR_DECL_LONG:
				if (!execVarDecl(interpreter, opcode == TOY_OP_VAR_DECL_LONG)) {
					return;
				}
			break;

			case TOY_OP_FN_DECL:
			case TOY_OP_FN_DECL_LONG:
				if (!execFnDecl(interpreter, opcode == TOY_OP_FN_DECL_LONG)) {
					return;
				}
			break;

			case TOY_OP_VAR_ASSIGN:
				if (!execVarAssign(interpreter)) {
					return;
				}
			break;

			case TOY_OP_TYPE_CAST:
				if (!execValCast(interpreter)) {
					return;
				}
			break;

			case TOY_OP_TYPE_OF:
				if (!execTypeOf(interpreter)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_EQUAL:
				if (!execCompareEqual(interpreter, false)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_NOT_EQUAL:
				if (!execCompareEqual(interpreter, true)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_LESS:
				if (!execCompareLess(interpreter, false)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_LESS_EQUAL:
				if (!execCompareLessEqual(interpreter, false)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_GREATER:
				if (!execCompareLess(interpreter, true)) {
					return;
				}
			break;

			case TOY_OP_COMPARE_GREATER_EQUAL:
				if (!execCompareLessEqual(interpreter, true)) {
					return;
				}
			break;

			case TOY_OP_INVERT:
				if (!execInvert(interpreter)) {
					return;
				}
			break;

			case TOY_OP_AND:
				if (!execAnd(interpreter)) {
					return;
				}
			break;

			case TOY_OP_OR:
				if (!execOr(interpreter)) {
					return;
				}
			break;

			case TOY_OP_JUMP:
				if (!execJump(interpreter)) {
					return;
				}
			break;

			case TOY_OP_IF_FALSE_JUMP:
				if (!execFalseJump(interpreter)) {
					return;
				}
			break;

			case TOY_OP_FN_CALL:
				if (!execFnCall(interpreter, false)) {
					return;
				}
			break;

			case TOY_OP_DOT:
				if (!execFnCall(interpreter, true)) { //compensate for the out-of-order arguments
					return;
				}
			break;

			case TOY_OP_FN_RETURN:
				if (!execFnReturn(interpreter)) {
					return;
				}
			break;

			case TOY_OP_IMPORT:
				if (!execImport(interpreter)) {
					return;
				}
			break;

			case TOY_OP_INDEX:
				if (!execIndex(interpreter, false)) {
					return;
				}
			break;

			case TOY_OP_INDEX_ASSIGN_INTERMEDIATE:
				if (!execIndex(interpreter, true)) {
					return;
				}
				intermediateAssignDepth++;
			break;

			case TOY_OP_INDEX_ASSIGN:
				if (!execIndexAssign(interpreter, intermediateAssignDepth)) {
					return;
				}
				intermediateAssignDepth = 0;
			break;

			case TOY_OP_POP_STACK:
				while (interpreter->stack.count > 0) {
					Toy_freeLiteral(Toy_popLiteralArray(&interpreter->stack));
				}
			break;

			default:
				interpreter->errorOutput("Unknown opcode found, terminating\n");
				return;
		}

		opcode = readByte(interpreter->bytecode, &interpreter->count);
	}
}

static void readInterpreterSections(Toy_Interpreter* interpreter) {
	//data section
	const unsigned short literalCount = readShort(interpreter->bytecode, &interpreter->count);

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		printf(TOY_CC_NOTICE "Reading %d literals\n" TOY_CC_RESET, literalCount);
	}
#endif

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = readByte(interpreter->bytecode, &interpreter->count);

		switch(literalType) {
			case TOY_LITERAL_NULL:
				//read the null
				Toy_pushLiteralArray(&interpreter->literalCache, TOY_TO_NULL_LITERAL);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(null)\n");
				}
#endif
			break;

			case TOY_LITERAL_BOOLEAN: {
				//read the booleans
				const bool b = readByte(interpreter->bytecode, &interpreter->count);
				Toy_Literal literal = TOY_TO_BOOLEAN_LITERAL(b);
				Toy_pushLiteralArray(&interpreter->literalCache, literal);
				Toy_freeLiteral(literal);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(boolean %s)\n", b ? "true" : "false");
				}
#endif
			}
			break;

			case TOY_LITERAL_INTEGER: {
				const int d = readInt(interpreter->bytecode, &interpreter->count);
				Toy_Literal literal = TOY_TO_INTEGER_LITERAL(d);
				Toy_pushLiteralArray(&interpreter->literalCache, literal);
				Toy_freeLiteral(literal);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(integer %d)\n", d);
				}
#endif
			}
			break;

			case TOY_LITERAL_FLOAT: {
				const float f = readFloat(interpreter->bytecode, &interpreter->count);
				Toy_Literal literal = TOY_TO_FLOAT_LITERAL(f);
				Toy_pushLiteralArray(&interpreter->literalCache, literal);
				Toy_freeLiteral(literal);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(float %f)\n", f);
				}
#endif
			}
			break;

			case TOY_LITERAL_STRING: {
				const char* s = readString(interpreter->bytecode, &interpreter->count);
				int length = strlen(s);
				Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(s, length));
				Toy_pushLiteralArray(&interpreter->literalCache, literal);
				Toy_freeLiteral(literal);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(string \"%s\")\n", s);
				}
#endif
			}
			break;

			case TOY_LITERAL_ARRAY_INTERMEDIATE:
			case TOY_LITERAL_ARRAY: {
				Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
				Toy_initLiteralArray(array);

				unsigned short length = readShort(interpreter->bytecode, &interpreter->count);

				//read each index, then unpack the value from the existing literal cache
				for (int i = 0; i < length; i++) {
					int index = readShort(interpreter->bytecode, &interpreter->count);
					Toy_pushLiteralArray(array, interpreter->literalCache.literals[index]);
				}

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(array ");
					Toy_Literal literal = TOY_TO_ARRAY_LITERAL(array);
					Toy_printLiteral(literal);
					printf(")\n");
				}
#endif

				//finally, push the array proper
				Toy_Literal literal = TOY_TO_ARRAY_LITERAL(array);
				Toy_pushLiteralArray(&interpreter->literalCache, literal); //copied

				Toy_freeLiteralArray(array);
				TOY_FREE(Toy_LiteralArray, array);
			}
			break;

			case TOY_LITERAL_DICTIONARY_INTERMEDIATE:
			case TOY_LITERAL_DICTIONARY: {
				Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
				Toy_initLiteralDictionary(dictionary);

				unsigned short length = readShort(interpreter->bytecode, &interpreter->count);

				//read each index, then unpack the value from the existing literal cache
				for (int i = 0; i < length / 2; i++) {
					int key = readShort(interpreter->bytecode, &interpreter->count);
					int val = readShort(interpreter->bytecode, &interpreter->count);
					Toy_setLiteralDictionary(dictionary, interpreter->literalCache.literals[key], interpreter->literalCache.literals[val]);
				}

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(dictionary ");
					Toy_Literal literal = TOY_TO_DICTIONARY_LITERAL(dictionary);
					Toy_printLiteral(literal);
					printf(")\n");
				}
#endif

				//finally, push the dictionary proper
				Toy_Literal literal = TOY_TO_DICTIONARY_LITERAL(dictionary);
				Toy_pushLiteralArray(&interpreter->literalCache, literal); //copied

				Toy_freeLiteralDictionary(dictionary);
				TOY_FREE(Toy_LiteralDictionary, dictionary);
			}
			break;

			case TOY_LITERAL_FUNCTION: {
				//read the index
				unsigned short index = readShort(interpreter->bytecode, &interpreter->count);
				Toy_Literal literal = TOY_TO_INTEGER_LITERAL(index);

				//change the type, to read it PROPERLY below
				literal.type = TOY_LITERAL_FUNCTION_INTERMEDIATE;

				//push to the literal cache
				Toy_pushLiteralArray(&interpreter->literalCache, literal);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(function)\n");
				}
#endif
			}
			break;

			case TOY_LITERAL_IDENTIFIER: {
				const char* str = readString(interpreter->bytecode, &interpreter->count);

				int length = strlen(str);
				Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefStringLength(str, length));

				Toy_pushLiteralArray(&interpreter->literalCache, identifier);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(identifier %s (hash: %x))\n", Toy_toCString(TOY_AS_IDENTIFIER(identifier)), identifier.as.identifier.hash);
				}
#endif

				Toy_freeLiteral(identifier);
			}
			break;

			case TOY_LITERAL_TYPE: {
				//what the literal is
				Toy_LiteralType literalType = (Toy_LiteralType)readByte(interpreter->bytecode, &interpreter->count);
				unsigned char constant = readByte(interpreter->bytecode, &interpreter->count);

				Toy_Literal typeLiteral = TOY_TO_TYPE_LITERAL(literalType, constant);

				//save the type
				Toy_pushLiteralArray(&interpreter->literalCache, typeLiteral);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(type ");
					Toy_printLiteral(typeLiteral);
					printf(")\n");
				}
#endif
			}
			break;

			case TOY_LITERAL_TYPE_INTERMEDIATE: {
				//what the literal represents
				Toy_LiteralType literalType = (Toy_LiteralType)readByte(interpreter->bytecode, &interpreter->count);
				unsigned char constant = readByte(interpreter->bytecode, &interpreter->count);

				Toy_Literal typeLiteral = TOY_TO_TYPE_LITERAL(literalType, constant);

				//if it's an array type
				if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ARRAY) {
					unsigned short vt = readShort(interpreter->bytecode, &interpreter->count);

					TOY_TYPE_PUSH_SUBTYPE(&typeLiteral, Toy_copyLiteral(interpreter->literalCache.literals[vt]));
				}

				if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_DICTIONARY) {
					unsigned short kt = readShort(interpreter->bytecode, &interpreter->count);
					unsigned short vt = readShort(interpreter->bytecode, &interpreter->count);

					TOY_TYPE_PUSH_SUBTYPE(&typeLiteral, Toy_copyLiteral(interpreter->literalCache.literals[kt]));
					TOY_TYPE_PUSH_SUBTYPE(&typeLiteral, Toy_copyLiteral(interpreter->literalCache.literals[vt]));
				}

				//save the type
				Toy_pushLiteralArray(&interpreter->literalCache, typeLiteral); //copied

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(type ");
					Toy_printLiteral(typeLiteral);
					printf(")\n");
				}
#endif

				Toy_freeLiteral(typeLiteral);
			}
			break;

			case TOY_LITERAL_INDEX_BLANK:
				//read the blank
				Toy_pushLiteralArray(&interpreter->literalCache, TOY_TO_INDEX_BLANK_LITERAL);

#ifndef TOY_EXPORT
				if (Toy_commandLine.verbose) {
					printf("(blank)\n");
				}
#endif
			break;
		}
	}

	consumeByte(interpreter, TOY_OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the literal section

	//read the function metadata
	int functionCount = readShort(interpreter->bytecode, &interpreter->count);
	int functionSize = readShort(interpreter->bytecode, &interpreter->count); //might not be needed

	//read in the functions
	for (int i = 0; i < interpreter->literalCache.count; i++) {
		if (interpreter->literalCache.literals[i].type == TOY_LITERAL_FUNCTION_INTERMEDIATE) {
			//get the size of the function
			size_t size = (size_t)readShort(interpreter->bytecode, &interpreter->count);

			//assert that the last memory slot is function end
			if (interpreter->bytecode[interpreter->count + size - 1] != TOY_OP_FN_END) {
				interpreter->errorOutput("[internal] Failed to find function end");
				return;
			}

			//copies internally, since functions can exist independant of literalCache
			interpreter->literalCache.literals[i] = TOY_TO_FUNCTION_LITERAL(Toy_createRefFunction(interpreter->bytecode + interpreter->count, size));

			interpreter->count += size;
		}
	}

	consumeByte(interpreter, TOY_OP_SECTION_END, interpreter->bytecode, &interpreter->count); //terminate the function section
}

//exposed functions
void Toy_initInterpreter(Toy_Interpreter* interpreter) {
	interpreter->hooks = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
	Toy_initLiteralDictionary(interpreter->hooks);

	//set up the output streams
	Toy_setInterpreterPrint(interpreter, printWrapper);
	Toy_setInterpreterAssert(interpreter, assertWrapper);
	Toy_setInterpreterError(interpreter, errorWrapper);

	interpreter->scope = NULL;
	Toy_resetInterpreter(interpreter);
}

void Toy_runInterpreter(Toy_Interpreter* interpreter, const unsigned char* bytecode, size_t length) {
	//initialize here instead of initInterpreter()
	Toy_initLiteralArray(&interpreter->literalCache);
	interpreter->bytecode = NULL;
	interpreter->length = 0;
	interpreter->count = 0;
	interpreter->codeStart = -1;

	Toy_initLiteralArray(&interpreter->stack);

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
		Toy_freeLiteralArray(&interpreter->literalCache); //automatically inits
	}

	//header section
	const unsigned char major = readByte(interpreter->bytecode, &interpreter->count);
	const unsigned char minor = readByte(interpreter->bytecode, &interpreter->count);
	const unsigned char patch = readByte(interpreter->bytecode, &interpreter->count);

	if (major != TOY_VERSION_MAJOR || minor > TOY_VERSION_MINOR) {
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, TOY_MAX_STRING_LENGTH, "Interpreter/bytecode version mismatch (expected %d.%d.%d or earlier, given %d.%d.%d)\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, major, minor, patch);
		interpreter->errorOutput(buffer);
		return;
	}

	const char* build = readString(interpreter->bytecode, &interpreter->count);

#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		if (strncmp(build, TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD))) {
			printf(TOY_CC_WARN "Warning: interpreter/bytecode build mismatch\n" TOY_CC_RESET);
		}
	}
#endif

	consumeByte(interpreter, TOY_OP_SECTION_END, interpreter->bytecode, &interpreter->count);

	//read the sections of the bytecode
	readInterpreterSections(interpreter);

	//code section
#ifndef TOY_EXPORT
	if (Toy_commandLine.verbose) {
		printf(TOY_CC_NOTICE "executing bytecode\n" TOY_CC_RESET);
	}
#endif

	//execute the interpreter
	execInterpreter(interpreter);

	//BUGFIX: clear the stack (for repl - stack must be balanced)
	while(interpreter->stack.count > 0) {
		Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);
		Toy_freeLiteral(lit);
	}

	//free the bytecode immediately after use TODO: because why?
	TOY_FREE_ARRAY(unsigned char, interpreter->bytecode, interpreter->length);

	//free the associated data
	Toy_freeLiteralArray(&interpreter->literalCache);
	Toy_freeLiteralArray(&interpreter->stack);
}

void Toy_resetInterpreter(Toy_Interpreter* interpreter) {
	//free the interpreter scope
	while(interpreter->scope != NULL) {
		interpreter->scope = Toy_popScope(interpreter->scope);
	}

	//prep the scope
	interpreter->scope = Toy_pushScope(NULL);

	//globally available functions
	Toy_injectNativeFn(interpreter, "set", Toy_private_set);
	Toy_injectNativeFn(interpreter, "get", Toy_private_get);
	Toy_injectNativeFn(interpreter, "push", Toy_private_push);
	Toy_injectNativeFn(interpreter, "pop", Toy_private_pop);
	Toy_injectNativeFn(interpreter, "length", Toy_private_length);
	Toy_injectNativeFn(interpreter, "clear", Toy_private_clear);
}

void Toy_freeInterpreter(Toy_Interpreter* interpreter) {
	//free the interpreter scope
	while(interpreter->scope != NULL) {
		interpreter->scope = Toy_popScope(interpreter->scope);
	}

	if (interpreter->hooks) {
		Toy_freeLiteralDictionary(interpreter->hooks);
		TOY_FREE(Toy_LiteralDictionary, interpreter->hooks);
	}

	interpreter->hooks = NULL;
}
