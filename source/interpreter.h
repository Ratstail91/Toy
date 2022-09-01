#pragma once

#include "opcodes.h"

#include "literal.h"
#include "literal_array.h"
#include "literal_dictionary.h"
#include "scope.h"

typedef void (*PrintFn)(const char*);

//the interpreter acts depending on the bytecode instructions
typedef struct Interpreter {
	//input
	unsigned char* bytecode;
	int length;
	int count;
	int codeStart; //for jumps
	LiteralArray literalCache; //read-only - built from the bytecode, refreshed each time new bytecode is provided

	//operation
	Scope* scope;
	LiteralArray stack;

	//output
	// LiteralDictionary exports; //TODO: read-write - interface with Toy from C
	PrintFn printOutput;
	PrintFn assertOutput;

	bool panic;
} Interpreter;

//for native function API
typedef int (*NativeFn)(Interpreter* interpreter, LiteralArray* arguments);
bool injectNativeFn(Interpreter* interpreter, char* name, NativeFn func);
bool parseIdentifierToValue(Interpreter* interpreter, Literal* literalPtr);

//init & free
void initInterpreter(Interpreter* interpreter);
void freeInterpreter(Interpreter* interpreter);

//utilities for the host program
void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput);
void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput);

void runInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length);
